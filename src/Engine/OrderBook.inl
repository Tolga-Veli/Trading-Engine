#pragma once

#ifndef ORDERBOOK_HPP
#include "OrderBook.hpp"
#endif

#include "Core/Order.hpp"
#include "Core/OrderBookSnapshot.hpp"
#include "Core/Trade.hpp"
#include "Core/globals.hpp"

namespace ob::engine {

template <class MatchingStrategy>
ErrorCode OrderBook<MatchingStrategy>::InternalAddOrder(
    ClientID clientID, Price price, Quantity quantity, Side side,
    OrderType order_type, TimeInForce tif, MatchType match_type,
    Flags flags) noexcept {

  if (match_type != MatchType::Standard) {
    HERMES_ERROR(
        "No implementation of matching types different than standard!");
    return ErrorCode::InvalidRequest;
  }

  if (tif == TimeInForce::FillOrKill) [[unlikely]] {
    Quantity available = 0;

    if (side == Side::Buy) {
      for (const auto &[ask_price, level_data] : m_Asks) {
        if (order_type != OrderType::Market && price < ask_price)
          break;

        available += level_data.volume;
        if (available >= quantity)
          break;
      }
    } else {
      for (const auto &[bid_price, level_data] : m_Bids) {
        if (order_type != OrderType::Market && price > bid_price)
          break;

        available += level_data.volume;
        if (available >= quantity)
          break;
      }
    }

    if (quantity > available) [[unlikely]]
      return ErrorCode::InsufficientLiquidity;
  }

  // PostOnly pre-check (Limit orders only)
  if (order_type == OrderType::Limit &&
      (flags & Flags::PostOnly) == Flags::PostOnly) [[unlikely]] {

    bool match = false;
    if (side == Side::Buy && !m_Asks.empty() && price >= m_Asks.begin()->first)
      match = true;
    if (side == Side::Sell && !m_Bids.empty() && price <= m_Bids.begin()->first)
      match = true;

    if (match) [[unlikely]]
      return ErrorCode::PostOnlyViolation;
  }

  const OrderID new_id = m_OrderCounter++;
  Order order{new_id,     clientID, price,      quantity, side,
              order_type, tif,      match_type, flags};

  m_MatchingStrategy.Match(order, *this);

  if (order.GetRemainingQuantity() == 0) [[likely]]
    return ErrorCode::Success;

  bool shouldRestOnBook = false;
  switch (tif) {
  case TimeInForce::ImmediateOrCancel:
  case TimeInForce::FillOrKill:
  case TimeInForce::FillAndKill:
    shouldRestOnBook = false;
    break;
  case TimeInForce::DayOrder:
  case TimeInForce::GoodTillCancelled:
    if (order_type == OrderType::Market) [[unlikely]]
      return ErrorCode::InvalidOrderCombination;

    shouldRestOnBook = true;
    break;
  case TimeInForce::GoodTillDate:
  case TimeInForce::AtTheOpening:
    return ErrorCode::NotImplemented;
  default:
    return ErrorCode::UnsupportedTimeInForce;
  }

  if (!shouldRestOnBook)
    return ErrorCode::Success;

  if (side == Side::Buy) {
    auto [level_it, inserted] = m_Bids.try_emplace(price, LevelData{&m_Pool});

    const Quantity remaining = order.GetRemainingQuantity();
    if ((flags & Flags::Hidden) == Flags::None) [[likely]]
      level_it->second.volume += remaining;

    auto &list = level_it->second.orders;
    list.emplace_back(std::move(order));
    m_Orders.emplace(new_id, OrderEntry{std::prev(list.end()), level_it});

  } else {
    auto [level_it, inserted] = m_Asks.try_emplace(price, LevelData{&m_Pool});

    const Quantity remaining = order.GetRemainingQuantity();
    if ((flags & Flags::Hidden) == Flags::None) [[likely]]
      level_it->second.volume += remaining;

    auto &list = level_it->second.orders;
    list.emplace_back(std::move(order));
    m_Orders.emplace(new_id, OrderEntry{std::prev(list.end()), level_it});
  }

  return ErrorCode::Success;
}

template <class MatchingStrategy>
ErrorCode OrderBook<MatchingStrategy>::InternalModifyOrder(
    ClientID clientID, OrderID orderID, Price new_price,
    Quantity new_quantity) noexcept {

  auto it = m_Orders.find(orderID);
  if (it == m_Orders.end()) [[unlikely]]
    return ErrorCode::InvalidModify;

  const auto list_it = it->second.list_it;
  const auto level_it = it->second.level_it;

  const Price old_price = list_it->GetPrice();
  const Quantity old_quantity = list_it->GetRemainingQuantity();

  if (list_it->GetClientID() != clientID) [[unlikely]]
    return ErrorCode::Unauthorized;

  const Side side = list_it->GetSide();
  const OrderType type = list_it->GetOrderType();
  const TimeInForce tif = list_it->GetTimeInForce();
  const MatchType match_type = list_it->GetMatchType();
  const Flags flags = list_it->GetFlags();

  if (new_price != old_price || new_quantity > old_quantity) {
    if (const auto code = InternalCancelOrder(clientID, orderID);
        code != ErrorCode::Success) [[unlikely]]
      return ErrorCode::InvalidModify;

    if (const auto code = InternalAddOrder(clientID, new_price, new_quantity,
                                           side, type, tif, match_type, flags);
        code != ErrorCode::Success)
      return ErrorCode::InvalidModify;

  } else if (new_quantity < old_quantity) {
    const Quantity diff = list_it->UpdateQuantity(new_quantity);
    if ((flags & Flags::Hidden) == Flags::None) [[likely]]
      level_it->second.volume -= diff;
  }

  return ErrorCode::Success;
}

template <class MatchingStrategy>
ErrorCode
OrderBook<MatchingStrategy>::InternalCancelOrder(ClientID clientID,
                                                 OrderID orderID) noexcept {
  auto entry_it = m_Orders.find(orderID);
  if (entry_it == m_Orders.end()) [[unlikely]]
    return ErrorCode::InvalidCancel;

  const auto list_it = entry_it->second.list_it;
  const auto level_it = entry_it->second.level_it;
  const Side side = list_it->GetSide();
  const Flags flags = list_it->GetFlags();

  if (list_it->GetClientID() != clientID) [[unlikely]]
    return ErrorCode::Unauthorized;

  if ((flags & Flags::Hidden) == Flags::None) [[likely]]
    level_it->second.volume -= list_it->GetRemainingQuantity();

  level_it->second.orders.erase(list_it);
  m_Orders.erase(entry_it);

  if (level_it->second.orders.empty()) [[unlikely]] {
    if (side == Side::Buy)
      m_Bids.erase(level_it);
    else
      m_Asks.erase(level_it);
  }

  return ErrorCode::Success;
}

template <class MatchingStrategy>
const Order *
OrderBook<MatchingStrategy>::FindOrder(const OrderID &orderID) const noexcept {
  const auto it = m_Orders.find(orderID);
  if (it == m_Orders.end()) [[unlikely]]
    return nullptr;
  else
    return &(*it->second.list_it);
}

template <class MatchingStrategy>
Order *OrderBook<MatchingStrategy>::GetBestBid() noexcept {
  if (m_Bids.empty()) [[unlikely]]
    return nullptr;
  else
    return &(m_Bids.begin()->second.orders.front());
}

template <class MatchingStrategy>
Order *OrderBook<MatchingStrategy>::GetBestAsk() noexcept {
  if (m_Asks.empty()) [[unlikely]]
    return nullptr;
  else
    return &(m_Asks.begin()->second.orders.front());
}

template <class MatchingStrategy>
Quantity
OrderBook<MatchingStrategy>::GetBidVolumeAtPrice(Price price) const noexcept {
  const auto it = m_Bids.find(price);
  if (it != m_Bids.end())
    return it->second.volume;
  else
    return Quantity{0};
}

template <class MatchingStrategy>
Quantity
OrderBook<MatchingStrategy>::GetAskVolumeAtPrice(Price price) const noexcept {
  const auto it = m_Asks.find(price);
  if (it != m_Asks.end())
    return it->second.volume;
  else
    return Quantity{0};
}

template <class MatchingStrategy>
OrderBookSnapshot
OrderBook<MatchingStrategy>::GetSnapshot(uint32_t depth) const noexcept {
  OrderBookSnapshot snapshot;
  snapshot.bids.reserve(depth), snapshot.asks.reserve(depth);

  u32 curr = 0;
  for (const auto &[price, level] : m_Bids) {
    if (++curr > depth)
      break;
    snapshot.bids.push_back({price, level.volume});
  }

  curr = 0;
  for (const auto &[price, level] : m_Asks) {
    if (++curr > depth)
      break;
    snapshot.asks.push_back({price, level.volume});
  }
  return snapshot;
}

template <class MatchingStrategy>
void OrderBook<MatchingStrategy>::RecordFill(OrderID makerOrderID,
                                             OrderID takerOrderID, Price price,
                                             Quantity quantity, Side takerSide,
                                             MatchType match_type) noexcept {
  if (takerSide == Side::Buy) {
    auto it = m_Bids.find(price);
    if (it != m_Bids.end()) [[likely]] {
      it->second.volume -= quantity;

      if (it->second.volume == 0 && it->second.orders.empty()) [[unlikely]]
        m_Bids.erase(it);
    }

  } else {
    auto it = m_Asks.find(price);
    if (it != m_Asks.end()) [[likely]] {
      it->second.volume -= quantity;

      if (it->second.volume == 0 && it->second.orders.empty()) [[unlikely]]
        m_Asks.erase(it);
    }
  }

  if (m_ScratchCounter < MaxEventsPerCommand) [[likely]] {
    t_EventScratch[m_ScratchCounter++] =
        Event{Trade{m_TradeCounter++, makerOrderID, takerOrderID, price,
                    quantity, takerSide, match_type}};
  }
}

template <class T>
void OrderBook<T>::Handle(const CommandTypes::AddOrder &cmd) noexcept {
  const auto code =
      InternalAddOrder(cmd.clientID, cmd.price, cmd.quantity, cmd.side,
                       cmd.order_type, cmd.tif, cmd.match_type, cmd.flags);

  if (code == ErrorCode::Success) [[likely]]
    t_EventScratch[m_ScratchCounter++] =
        Event{EventTypes::OrderAccepted{cmd.clientID, m_OrderCounter - 1}};
  else
    t_EventScratch[m_ScratchCounter++] =
        Event{EventTypes::OrderRejected{cmd.clientID, code}};
}

template <class T>
void OrderBook<T>::Handle(const CommandTypes::ModifyOrder &cmd) noexcept {
  const auto code = InternalModifyOrder(cmd.clientID, cmd.orderID,
                                        cmd.new_price, cmd.new_quantity);

  if (code == ErrorCode::Success) [[likely]]
    t_EventScratch[m_ScratchCounter++] =
        Event{EventTypes::ModifyAccepted{cmd.clientID, cmd.orderID}};
  else
    t_EventScratch[m_ScratchCounter++] =
        Event{EventTypes::ModifyRejected{cmd.clientID, cmd.orderID}};
}

template <class T>
void OrderBook<T>::Handle(const CommandTypes::CancelOrder &cmd) noexcept {
  const auto code = InternalCancelOrder(cmd.clientID, cmd.orderID);

  if (code == ErrorCode::Success) [[likely]]
    t_EventScratch[m_ScratchCounter++] =
        Event{EventTypes::CancelAccepted{cmd.clientID, cmd.orderID}};
  else
    t_EventScratch[m_ScratchCounter++] =
        Event{EventTypes::CancelRejected{cmd.clientID, cmd.orderID, code}};
}
} // namespace ob::engine
