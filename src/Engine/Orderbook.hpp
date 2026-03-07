#pragma once

#include <list>
#include <map>
#include <memory_resource>
#include <unordered_map>
#include <variant>

#include "Core/Commands.hpp"
#include "Core/Order.hpp"
#include "EventQueue.hpp"
#include "Matching/MatchingStrategy.hpp"

namespace ob::engine {
class OrderBook {
public:
  explicit OrderBook(EventQueue &queue,
                     std::pmr::unsynchronized_pool_resource &resource,
                     std::unique_ptr<IMatchingStrategy> strategy =
                         std::make_unique<FIFO_Matching>())
      : m_PoolResource(resource), m_Bids(&m_PoolResource),
        m_Asks(&m_PoolResource), m_Orders(&m_PoolResource), m_EventQueue(queue),
        m_MatchingStrategy(std::move(strategy)) {}

  ~OrderBook() {}

  OrderBook(const OrderBook &) = delete;
  OrderBook &operator=(const OrderBook &) = delete;
  OrderBook(OrderBook &&) = delete;
  OrderBook &operator=(OrderBook &&) = delete;

  void Apply(const Command &cmd) {
    std::visit([this](auto &&c) { Handle(c); }, cmd);
  }

  std::optional<Order> FindOrder(const OrderID &orderID) const noexcept;

  Order *GetBestBid() noexcept;
  Order *GetBestAsk() noexcept;

  Quantity GetBidVolumeAtPrice(Price price) const noexcept;
  Quantity GetAskVolumeAtPrice(Price price) const noexcept;

  std::size_t GetBidsDepth() const noexcept;
  std::size_t GetAsksDepth() const noexcept;

  engine::OrderBookSnapshot GetSnapshot(uint32_t depth) const noexcept;

  void PrintOrderBook() const;

  void AddOrder(const ClientID &clientID, const ClientOrderID &clientOrderID,
                const Price &price, const Quantity &quantity, const Side &side,
                const OrderType &order_type, const TimeInForce &tif,
                const Flags &flags);

  void ModifyOrder(const OrderID &orderID, const Price &new_price,
                   const Quantity &new_quantity);

  void CancelOrder(const OrderID &orderID);

private:
  struct OrderPointer {
    std::pmr::list<Order>::iterator list_iterator;
    std::pmr::map<Price, std::pmr::list<Order>>::iterator map_iterator;
  };

  std::pmr::unsynchronized_pool_resource &m_PoolResource;
  std::pmr::map<Price, std::pmr::list<Order>, std::greater<Price>> m_Bids;
  std::pmr::map<Price, std::pmr::list<Order>, std::less<Price>> m_Asks;
  std::pmr::unordered_map<OrderID, OrderPointer> m_Orders;

  EventQueue &m_EventQueue;
  std::unique_ptr<IMatchingStrategy> m_MatchingStrategy;

  void Handle(const std::monostate &empty) {}

  void Handle(const CommandTypes::AddOrder &add) {
    AddOrder(add.clientID, add.clientOrderID, add.price, add.quantity, add.side,
             add.order_type, add.tif, add.flags);
  }

  void Handle(const CommandTypes::ModifyOrder &modify) {
    ModifyOrder(modify.orderID, modify.new_price, modify.new_quantity);
  }

  void Handle(const CommandTypes::CancelOrder &cancel) {
    CancelOrder(cancel.orderID);
  }

  void Handle(const CommandTypes::RequestSnapshot &snapshot) {
    auto data = GetSnapshot(snapshot.depth);
    m_EventQueue.push(
        Event{EventTypes::SnapshotRequestAccepted{std::move(data)}});
  }
};
} // namespace ob::engine
