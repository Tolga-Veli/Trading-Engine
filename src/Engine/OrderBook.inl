#pragma once

#ifndef ORDERBOOK_HPP
#include "OrderBook.hpp" // This helps the LSP "see" the class
#endif

#include "Order.hpp"
#include "OrderBookSnapshot.hpp"
#include "globals.hpp"

#include <cassert>

namespace ob::engine {

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::AddOrder(
    const ClientID &clientID, const Price &price, const Quantity &quantity,
    const Side &side, const OrderType &order_type, const TimeInForce &tif,
    const Flags &flags) {

  m_OrderCounter++;
  Order order{m_OrderCounter, clientID,   price, quantity,
              side,           order_type, tif,   flags};

  m_MatchingStrategy.Match(order, *this);

  if (order.GetRemainingQuantity() == 0)
    return;

  if (side == Side::Buy) {
    auto [level_it, inserted] =
        m_Bids.try_emplace(price, LevelData{&m_PoolResource});
    level_it->second.volume += order.GetRemainingQuantity();
    auto &list = level_it->second.orders;

    list.emplace_back(std::move(order));
    m_Orders[m_OrderCounter] = {std::prev(list.end()), level_it};
  } else {
    auto [level_it, inserted] =
        m_Asks.try_emplace(price, LevelData{&m_PoolResource});
    level_it->second.volume += order.GetRemainingQuantity();
    auto &list = level_it->second.orders;

    list.emplace_back(std::move(order));
    m_Orders[m_OrderCounter] = {std::prev(list.end()), level_it};
  }

  m_EventQueue.push(EventTypes::OrderAccepted{clientID, m_OrderCounter});
  return;
}

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::ModifyOrder(const ClientID &clientID,
                                              const OrderID &orderID,
                                              const Price &new_price,
                                              const Quantity &new_quantity) {
  auto it = m_Orders.find(orderID);
  if (it == m_Orders.end()) {
    HERMES_WARN("OrderBook::ModifyOrder() trying to modify an order wtih ID:{} "
                "that doesn't exist",
                orderID);
    m_EventQueue.push(EventTypes::ModifyRejected{orderID, clientID,
                                                 ErrorCode::InvalidRequest});
    return;
  }

  auto &entry = it->second;
  auto list_it = entry.list_iterator;
  auto map_it = entry.map_iterator;

  const Price old_price = list_it->GetPrice();
  const Quantity old_quantity = list_it->GetRemainingQuantity();
  const Side side = list_it->GetSide();
  const OrderType type = list_it->GetOrderType();
  const TimeInForce tif = list_it->GetTimeInForce();
  const Flags flags = list_it->GetFlags();

  if (new_price != old_price || new_quantity > old_quantity) {
    CancelOrder(clientID, orderID);
    AddOrder(clientID, new_price, new_quantity, side, type, tif, flags);

    // new orderID
    m_EventQueue.push(EventTypes::ModifyAccepted{clientID, m_OrderCounter});

  } else if (new_quantity < old_quantity) {
    Quantity diff = list_it->UpdateQuantity(new_quantity);
    map_it->second.volume += diff;

    m_EventQueue.push(EventTypes::ModifyAccepted{clientID, orderID});
  }
}

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::CancelOrder(const ClientID &clientID,
                                              const OrderID &orderID) {
  auto entry_it = m_Orders.find(orderID);
  if (entry_it == m_Orders.end()) {
    HERMES_WARN("OrderBook::CancelOrder() trying to cancel an order that "
                "doesn't exist");
    m_EventQueue.push(EventTypes::CancelRejected{orderID, clientID,
                                                 ErrorCode::InvalidRequest});
    return;
  }

  auto &order_entry = entry_it->second;
  auto list_it = order_entry.list_iterator;
  auto map_it = order_entry.map_iterator;

  const Side side = list_it->GetSide();

  map_it->second.volume -= list_it->GetRemainingQuantity();

  auto &orderList = map_it->second.orders;
  orderList.erase(list_it);
  m_Orders.erase(entry_it);

  if (orderList.empty()) {
    if (side == Side::Buy)
      m_Bids.erase(map_it);
    else
      m_Asks.erase(map_it);
  }

  m_EventQueue.push(EventTypes::CancelAccepted{clientID, orderID});
}

template <class MatchingStrategy>
const Order *
OrderBook<MatchingStrategy>::FindOrder(const OrderID &orderID) const noexcept {
  auto entry_it = m_Orders.find(orderID);
  if (entry_it == m_Orders.end())
    return nullptr;
  return &(*entry_it->second.list_iterator);
}

template <class MatchingStrategy>
Order *OrderBook<MatchingStrategy>::GetBestBid() noexcept {
  if (m_Bids.empty())
    return nullptr;
  return &(m_Bids.begin()->second.orders.front());
}

template <class MatchingStrategy>
Order *OrderBook<MatchingStrategy>::GetBestAsk() noexcept {
  if (m_Asks.empty())
    return nullptr;
  return &(m_Asks.begin()->second.orders.front());
}

template <class MatchingStrategy>
Quantity
OrderBook<MatchingStrategy>::GetBidVolumeAtPrice(Price price) const noexcept {
  auto level_it = m_Bids.find(price);
  if (level_it == m_Bids.end())
    return 0;
  return level_it->second.volume;
}

template <class MatchingStrategy>
Quantity
OrderBook<MatchingStrategy>::GetAskVolumeAtPrice(Price price) const noexcept {
  auto level_it = m_Asks.find(price);
  if (level_it == m_Asks.end())
    return 0;
  return level_it->second.volume;
}

template <class MatchingStrategy>
OrderBookSnapshot
OrderBook<MatchingStrategy>::GetSnapshot(uint32_t depth) const noexcept {
  OrderBookSnapshot snapshot;
  snapshot.bids.reserve(depth), snapshot.asks.reserve(depth);

  u32 curr = 0;
  for (const auto &[price, level_data] : m_Bids) {
    if (++curr > depth)
      break;
    snapshot.bids.push_back({price, level_data.volume});
  }

  curr = 0;
  for (const auto &[price, level_data] : m_Asks) {
    if (++curr > depth)
      break;
    snapshot.asks.push_back({price, level_data.volume});
  }
  return snapshot;
}

} // namespace ob::engine
