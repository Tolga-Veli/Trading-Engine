#include "OrderBook.hpp"
#include <cassert>

namespace ob::engine {

void OrderBook::AddOrder(const ClientID &clientID,
                         const ClientOrderID &clientOrderID, const Price &price,
                         const Quantity &quantity, const Side &side,
                         const OrderType &order_type, const TimeInForce &tif,
                         const Flags &flags) {
  static OrderID orderID_counter = 1;
  const Order order{orderID_counter++, clientID, price, quantity, side,
                    order_type,        tif,      flags};

  if (m_Orders.count(order.GetOrderID()))
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
  m_MatchingStrategy->Match(*this);

  // TODO: Add Add Order event
}

void OrderBook::ModifyOrder(const OrderID &orderID, const Price &new_price,
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
  m_MatchingStrategy->Match(*this);

  // TODO: Add Modify Order event
}

void OrderBook::CancelOrder(const OrderID &orderID) {
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

std::optional<Order>
OrderBook::FindOrder(const OrderID &orderID) const noexcept {
  auto it = m_Orders.find(orderID);
  if (it == m_Orders.end())
    return std::nullopt;
  return *(it->second.list_iterator);
}

Order *OrderBook::GetBestBid() noexcept {
  if (m_Bids.empty())
    return nullptr;
  return &(m_Bids.begin()->second.front());
}

Order *OrderBook::GetBestAsk() noexcept {
  if (m_Asks.empty())
    return nullptr;
  return &(m_Asks.begin()->second.front());
}

Quantity OrderBook::GetBidVolumeAtPrice(Price price) const noexcept {
  Quantity total = 0;
  auto it = m_Bids.find(price);
  if (it != m_Bids.end())
    for (const auto &order : it->second)
      total += order.GetRemainingQuantity();

  return total;
}

Quantity OrderBook::GetAskVolumeAtPrice(Price price) const noexcept {
  Quantity total = 0;
  auto it = m_Asks.find(price);
  if (it != m_Asks.end())
    for (const auto &order : it->second)
      total += order.GetRemainingQuantity();

  return total;
}

std::size_t OrderBook::GetBidsDepth() const noexcept { return m_Bids.size(); }
std::size_t OrderBook::GetAsksDepth() const noexcept { return m_Asks.size(); }

OrderBookSnapshot OrderBook::GetSnapshot(uint32_t depth) const noexcept {
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

void OrderBook::PrintOrderBook() const {
  std::cout << "------------------\n";
  for (const auto &[_, op] : m_Orders)
    op.list_iterator->log();
  std::cout << "------------------\n";
}
} // namespace ob::engine
