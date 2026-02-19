#pragma once

#include "Core/Order.hpp"
#include "OrderBookSnapshot.hpp"

namespace ob::engine {
class OrderBook;

class OrderBookQuery {
public:
  explicit OrderBookQuery(engine::OrderBook &order_book)
      : m_Orderbook(order_book) {}

  OrderBookQuery(const OrderBookQuery &) = delete;
  OrderBookQuery &operator=(const OrderBookQuery &) = delete;
  OrderBookQuery(OrderBookQuery &&) = delete;
  OrderBookQuery &operator=(OrderBookQuery &&) = delete;

  bool HasOrders() const noexcept;
  std::optional<Order> FindOrder(const OrderID &orderID) const noexcept;

  const Order *GetBestBid() const noexcept;
  const Order *GetBestAsk() const noexcept;

  Quantity GetBidVolumeAtPrice(Price price) const noexcept;
  Quantity GetAskVolumeAtPrice(Price price) const noexcept;

  std::size_t GetBidsDepth() const noexcept;
  std::size_t GetAsksDepth() const noexcept;

  engine::OrderBookSnapshot GetSnapshot(uint32_t depth) const noexcept;

  void PrintOrderBook() const;

private:
  const engine::OrderBook &m_Orderbook;
};
} // namespace ob::engine
