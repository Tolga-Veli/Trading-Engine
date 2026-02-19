#pragma once

#include <list>
#include <map>
#include <memory_resource>
#include <unordered_map>
#include <variant>

#include "Commands.hpp"
#include "MatchingStrategy.hpp"
#include "OrderBookQuery.hpp"
#include "OrderPointer.hpp"

namespace ob::engine {
class OrderBookQuery;

class OrderBook {
public:
  friend class OrderBookQuery;

  explicit OrderBook(std::pmr::monotonic_buffer_resource &resource,
                     std::unique_ptr<IMatchingStrategy> strategy =
                         std::make_unique<FIFO_Matching>())
      : m_Resource(resource), m_Bids(&resource), m_Asks(&resource),
        m_Orders(&resource), m_QueryAPI(*this),
        m_MatchingStrategy(std::move(strategy)) {}

  ~OrderBook() {}

  OrderBook(const OrderBook &) = delete;
  OrderBook &operator=(const OrderBook &) = delete;
  OrderBook(OrderBook &&) = delete;
  OrderBook &operator=(OrderBook &&) = delete;

  void Apply(const Command &cmd) {
    std::visit([this](auto &&c) { Handle(c); }, cmd);
  }

  void Shutdown() noexcept { m_Running = false; };

  const OrderBookQuery &GetQueryAPI() const { return m_QueryAPI; }

  void AddOrder(const ClientID &clientID, const ClientOrderID &clientOrderID,
                const Price &price, const Quantity &quantity, const Side &side,
                const OrderType &order_type, const TimeInForce &tif,
                const Flags &flags);

  void ModifyOrder(const OrderID &orderID, const Price &new_price,
                   const Quantity &new_quantity);

  void CancelOrder(const OrderID &orderID);

private:
  bool m_Running = true;

  std::pmr::monotonic_buffer_resource &m_Resource;

  std::pmr::map<Price, std::pmr::list<Order>, std::greater<Price>> m_Bids;
  std::pmr::map<Price, std::pmr::list<Order>, std::less<Price>> m_Asks;
  std::pmr::unordered_map<OrderID, OrderPointer> m_Orders;

  OrderBookQuery m_QueryAPI;
  std::unique_ptr<IMatchingStrategy> m_MatchingStrategy;

  void Execute(const MatchResult &result);

  void Handle(const std::monostate &empty) {}
  void Handle(const cmd::AddOrder &add) {
    AddOrder(add.clientID, add.clientOrderID, add.price, add.quantity, add.side,
             add.order_type, add.tif, add.flags);
  }
  void Handle(const cmd::ModifyOrder &modify) {
    ModifyOrder(modify.orderID, modify.new_price, modify.new_quantity);
  }
  void Handle(const cmd::CancelOrder &cancel) { CancelOrder(cancel.orderID); }
};
} // namespace ob::engine
