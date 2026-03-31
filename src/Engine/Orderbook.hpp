#pragma once

#include <list>
#include <map>
#include <memory_resource>
#include <unordered_map>
#include <variant>

#include "Core/Commands.hpp"
#include "Core/Order.hpp"
#include "Core/OrderBookSnapshot.hpp"
#include "EventQueue.hpp"
#include "Matching/MatchingStrategy.hpp"

namespace ob::engine {
struct LevelData {
  Quantity volume = 0;
  std::pmr::list<Order> orders;

  LevelData() = delete;
  LevelData(std::pmr::memory_resource *mr) : orders(mr) {}
  ~LevelData() = default;
};

struct OrderEntry {
  std::pmr::list<Order>::iterator list_iterator;
  std::pmr::map<Price, LevelData>::iterator map_iterator;
};

template <class MatchingStrategy> class OrderBook {
  static_assert(
      requires(MatchingStrategy s, Order &order, OrderBook &book) {
        s.Match(order, book);
      }, "MatchingStrategy::Match must exist and accept an Order& and an"
         "OrderBook<MatchingStrategy>&");

public:
  explicit OrderBook(EventQueue &queue,
                     std::size_t pool_size = 32 * 1024 * 1024)
      : m_Buffer(pool_size), m_PoolResource(&m_Buffer), m_Bids(&m_PoolResource),
        m_Asks(&m_PoolResource), m_Orders(&m_PoolResource),
        m_MatchingStrategy() {}

  ~OrderBook() = default;

  OrderBook(const OrderBook &) = delete;
  OrderBook &operator=(const OrderBook &) = delete;
  OrderBook(OrderBook &&) = delete;
  OrderBook &operator=(OrderBook &&) = delete;

  ErrorCode AddOrder(const ClientID &clientID, const Price &price,
                     const Quantity &quantity, const Side &side,
                     const OrderType &order_type, const TimeInForce &tif,
                     const Flags &flags);

  ErrorCode ModifyOrder(const ClientID &clientID, const OrderID &orderID,
                        const Price &new_price, const Quantity &new_quantity);

  ErrorCode CancelOrder(const ClientID &clientID, const OrderID &orderID);

  [[nodiscard]] const Order *FindOrder(const OrderID &orderID) const noexcept;

  [[nodiscard]] Order *GetBestBid() noexcept;
  [[nodiscard]] Order *GetBestAsk() noexcept;

  Quantity GetBidVolumeAtPrice(Price price) const noexcept;
  Quantity GetAskVolumeAtPrice(Price price) const noexcept;

  [[nodiscard]] OrderBookSnapshot GetSnapshot(uint32_t depth) const noexcept;

  void RecordFill(const Price &price, const Quantity &quantity,
                  const Side &side) {
    if (side == Side::Buy) {
      auto it = m_Bids.find(price);
      if (it != m_Bids.end())
        it->second.volume -= quantity;
    } else {
      auto it = m_Asks.find(price);
      if (it != m_Asks.end())
        it->second.volume -= quantity;
    }
  }

  ErrorCode Apply(const Command &cmd) {
    return cmd.Decompose([this](auto &&arg) { Handle(arg); });
  }

private:
  std::pmr::monotonic_buffer_resource m_Buffer;
  std::pmr::unsynchronized_pool_resource m_PoolResource;

  std::pmr::map<Price, LevelData, std::greater<Price>> m_Bids;
  std::pmr::map<Price, LevelData, std::less<Price>> m_Asks;
  std::pmr::unordered_map<OrderID, OrderEntry> m_Orders;

  MatchingStrategy m_MatchingStrategy;

  OrderID m_OrderCounter = 0;

  void Handle(const std::monostate &empty) const noexcept {}

  ErrorCode Handle(const CommandTypes::AddOrder &order) {
    return AddOrder(order.clientID, order.price, order.quantity, order.side,
                    order.order_type, order.tif, order.flags, false);
  }

  ErrorCode Handle(const CommandTypes::ModifyOrder &order) {
    return ModifyOrder(order.clientID, order.orderID, order.new_price,
                       order.new_quantity);
  }

  ErrorCode Handle(const CommandTypes::CancelOrder &cancel) {
    return CancelOrder(cancel.clientID, cancel.orderID, false);
  }
};
} // namespace ob::engine

#include "OrderBook.inl"
