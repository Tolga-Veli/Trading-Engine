#pragma once

#ifndef ORDERBOOK_HPP
#include "OrderBook.hpp" // This helps the LSP "see" the class
#endif

#include "Order.hpp"
#include "OrderBookSnapshot.hpp"
#include "globals.hpp"

#include <cassert>
#include <optional>

namespace ob::engine {

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::AddOrder(
    const ClientID &clientID, const ClientOrderID &clientOrderID,
    const Price &price, const Quantity &quantity, const Side &side,
    const OrderType &order_type, const TimeInForce &tif, const Flags &flags) {

  static OrderID orderID_counter = 1;
  const Order order{orderID_counter++, clientID, price, quantity, side,
                    order_type,        tif,      flags};

  if (m_Orders.contains(order.GetOrderID()))
    return;

  OrderPointer op{};
  if (order.GetSide() == Side::Buy) {
    auto [it, inserted] = m_Bids.try_emplace(
        order.GetPrice(), std::pmr::list<Order>(&m_PoolResource));
    auto &list = it->second;

    list.push_back(std::move(order));
    op.list_iterator = std::prev(list.end());
    op.map_iterator = it;
  } else {
    auto [it, inserted] = m_Asks.try_emplace(
        order.GetPrice(), std::pmr::list<Order>(&m_PoolResource));
    auto &list = it->second;

    list.push_back(std::move(order));
    op.list_iterator = std::prev(list.end());
    op.map_iterator = it;
  }
  const OrderID id = op.list_iterator->GetOrderID();
  m_Orders[id] = op;
  m_MatchingStrategy.Match(*this);

  // TODO: Add Add Order event
}

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::ModifyOrder(const OrderID &orderID,
                                              const Price &new_price,
                                              const Quantity &new_quantity) {
  auto it = m_Orders.find(orderID);
  if (it == m_Orders.end())
    throw std::logic_error("ModifyOrder():: order not found!");

  auto &order = *(it->second.list_iterator);
  if (new_price != order.GetPrice())
    throw std::logic_error("ModifyOrder():: price change not supported!");

  if (new_quantity < order.GetFilledQuantity())
    throw std::logic_error("ModifyOrder():: new quantity < filled quantity!");

  order.ModifyOrder(new_price, new_quantity);
  m_MatchingStrategy.Match(*this);

  // TODO: Add Modify Order event
}

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::CancelOrder(const OrderID &orderID) {
  auto it = m_Orders.find(orderID);
  if (it == m_Orders.end())
    throw std::logic_error("CancelOrder():: order not not found");

  auto map_it = it->second.map_iterator;
  auto &orderList = map_it->second;

  Price price = it->second.list_iterator->GetPrice();
  Side side = it->second.list_iterator->GetSide();

  orderList.erase(it->second.list_iterator);
  m_Orders.erase(it);

  if (!orderList.empty())
    return;

  if (side == Side::Buy)
    m_Bids.erase(map_it);
  else
    m_Asks.erase(map_it);

  // TODO: Add Cancel Order event
}

template <class MatchingStrategy>
std::optional<Order>
OrderBook<MatchingStrategy>::FindOrder(const OrderID &orderID) const noexcept {
  auto it = m_Orders.find(orderID);
  if (it == m_Orders.end())
    return std::nullopt;
  return *(it->second.list_iterator);
}

template <class MatchingStrategy>
Order *OrderBook<MatchingStrategy>::GetBestBid() noexcept {
  if (m_Bids.empty())
    return nullptr;
  return &(m_Bids.begin()->second.front());
}

template <class MatchingStrategy>
Order *OrderBook<MatchingStrategy>::GetBestAsk() noexcept {
  if (m_Asks.empty())
    return nullptr;
  return &(m_Asks.begin()->second.front());
}

template <class MatchingStrategy>
Quantity
OrderBook<MatchingStrategy>::GetBidVolumeAtPrice(Price price) const noexcept {
  Quantity total = 0;
  auto it = m_Bids.find(price);
  if (it != m_Bids.end())
    for (const auto &order : it->second)
      total += order.GetRemainingQuantity();

  return total;
}

template <class MatchingStrategy>
Quantity
OrderBook<MatchingStrategy>::GetAskVolumeAtPrice(Price price) const noexcept {
  Quantity total = 0;
  auto it = m_Asks.find(price);
  if (it != m_Asks.end())
    for (const auto &order : it->second)
      total += order.GetRemainingQuantity();

  return total;
}

template <class MatchingStrategy>
std::size_t OrderBook<MatchingStrategy>::GetBidsDepth() const noexcept {
  return m_Bids.size();
}

template <class MatchingStrategy>
std::size_t OrderBook<MatchingStrategy>::GetAsksDepth() const noexcept {
  return m_Asks.size();
}

template <class MatchingStrategy>
OrderBookSnapshot
OrderBook<MatchingStrategy>::GetSnapshot(uint32_t depth) const noexcept {
  uint32_t curr_depth = 0;
  std::vector<std::pair<Price, Quantity>> bids, asks;
  for (const auto &[price, list] : m_Bids) {
    if (++curr_depth > depth)
      break;

    Quantity total_quantity = 0;
    for (const auto &order : list)
      total_quantity += order.GetRemainingQuantity();

    bids.push_back({price, total_quantity});
  }

  curr_depth = 0;
  for (const auto &[price, list] : m_Asks) {
    if (++curr_depth > depth)
      break;

    Quantity total_quantity = 0;
    for (const auto &order : list)
      total_quantity += order.GetRemainingQuantity();

    asks.push_back({price, total_quantity});
  }

  return {bids, asks};
}

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::PrintOrderBook() const {
  std::cout << "------------------\n";
  for (const auto &[_, op] : m_Orders)
    op.list_iterator->log();
  std::cout << "------------------\n";
}
} // namespace ob::engine
