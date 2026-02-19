#include "OrderBookQuery.hpp"
#include "OrderBook.hpp"

namespace ob::engine {
bool OrderBookQuery::HasOrders() const noexcept {
  return !m_Orderbook.m_Bids.empty() && !m_Orderbook.m_Asks.empty();
}

std::optional<Order>
OrderBookQuery::FindOrder(const OrderID &orderID) const noexcept {
  auto it = m_Orderbook.m_Orders.find(orderID);
  if (it == m_Orderbook.m_Orders.end())
    return std::nullopt;
  return *(it->second.list_iterator);
}

const Order *OrderBookQuery::GetBestBid() const noexcept {
  if (m_Orderbook.m_Bids.empty())
    return nullptr;
  return &(m_Orderbook.m_Bids.begin()->second.front());
}

const Order *OrderBookQuery::GetBestAsk() const noexcept {
  if (m_Orderbook.m_Asks.empty())
    return nullptr;
  return &(m_Orderbook.m_Asks.begin()->second.front());
}

Quantity OrderBookQuery::GetBidVolumeAtPrice(Price price) const noexcept {
  Quantity total = 0;
  auto it = m_Orderbook.m_Bids.find(price);
  if (it != m_Orderbook.m_Bids.end())
    for (const auto &order : it->second)
      total += order.GetRemainingQuantity();

  return total;
}

Quantity OrderBookQuery::GetAskVolumeAtPrice(Price price) const noexcept {
  Quantity total = 0;
  auto it = m_Orderbook.m_Asks.find(price);
  if (it != m_Orderbook.m_Asks.end())
    for (const auto &order : it->second)
      total += order.GetRemainingQuantity();

  return total;
}

std::size_t OrderBookQuery::GetBidsDepth() const noexcept {
  return m_Orderbook.m_Bids.size();
}
std::size_t OrderBookQuery::GetAsksDepth() const noexcept {
  return m_Orderbook.m_Asks.size();
}

OrderBookSnapshot OrderBookQuery::GetSnapshot(uint32_t depth) const noexcept {
  uint32_t curr_depth = 0;
  std::vector<std::pair<Price, Quantity>> bids, asks;
  for (const auto &[price, list] : m_Orderbook.m_Bids) {
    if (++curr_depth > depth)
      break;

    Quantity total_quantity = 0;
    for (const auto &order : list)
      total_quantity += order.GetRemainingQuantity();

    bids.push_back({price, total_quantity});
  }

  curr_depth = 0;
  for (const auto &[price, list] : m_Orderbook.m_Asks) {
    if (++curr_depth > depth)
      break;

    Quantity total_quantity = 0;
    for (const auto &order : list)
      total_quantity += order.GetRemainingQuantity();

    asks.push_back({price, total_quantity});
  }

  return {bids, asks};
}

void OrderBookQuery::PrintOrderBook() const {
  std::cout << "------------------\n";
  for (const auto &[orderID, OrderPointer] : m_Orderbook.m_Orders)
    OrderPointer.list_iterator->info();
  std::cout << "------------------\n";
}
} // namespace ob::engine
