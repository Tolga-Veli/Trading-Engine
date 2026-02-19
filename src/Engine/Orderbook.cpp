#include "OrderBook.hpp"
#include <cassert>

namespace ob::engine {

void OrderBook::AddOrder(const ClientID &clientID,
                         const ClientOrderID &clientOrderID, const Price &price,
                         const Quantity &quantity, const Side &side,
                         const OrderType &order_type, const TimeInForce &tif,
                         const Flags &flags) {
  static OrderID orderID_counter = 1;
  const Order order{orderID_counter, clientID, price, quantity, side,
                    order_type,      tif,      flags};

  if (m_Orders.count(order.GetOrderID()))
    return;

  OrderPointer op{};
  if (order.GetSide() == Side::Buy) {
    auto [it, inserted] = m_Bids.try_emplace(
        order.GetPrice(), std::pmr::list<Order>(&m_Resource));
    auto &list = it->second;

    list.push_back(std::move(order));
    op.list_iterator = std::prev(list.end());
    op.map_iterator = it;
  } else {
    auto [it, inserted] = m_Asks.try_emplace(
        order.GetPrice(), std::pmr::list<Order>(&m_Resource));
    auto &list = it->second;

    list.push_back(std::move(order));
    op.list_iterator = std::prev(list.end());
    op.map_iterator = it;
  }
  const OrderID id = op.list_iterator->GetOrderID();
  m_Orders[id] = op;

  auto intent = m_MatchingStrategy->Match(order, m_QueryAPI);
  Execute(intent);

  // TODO: Add Add Order event
}

void OrderBook::ModifyOrder(const OrderID &orderID, const Price &new_price,
                            const Quantity &new_quantity) {
  auto it = m_Orders.find(orderID);
  auto &order = *(it->second.list_iterator);
  if (new_price != order.GetPrice() || new_quantity > order.GetFilledQuantity())
    throw std::logic_error("ModifyOrder:: The new quantity must be >= then the "
                           "order's filled quantity");

  order.ModifyOrder(new_price, new_quantity);
  auto intent = m_MatchingStrategy->Match(order, m_QueryAPI);
  Execute(intent);

  // TODO: Add Modify Order event
}

void OrderBook::CancelOrder(const OrderID &orderID) {
  auto it = m_Orders.find(orderID);
  auto &iter = it->second.list_iterator;
  auto &[_, orderList] = *(it->second.map_iterator);

  Price price = iter->GetPrice();
  Side side = iter->GetSide();

  orderList.erase(iter);
  m_Orders.erase(it);

  if (!orderList.empty())
    return;

  if (side == Side::Buy)
    m_Bids.erase(price);
  else
    m_Asks.erase(price);

  // TODO: Add Cancel Order event
}

void OrderBook::Execute(const MatchResult &result) {
  const auto &[trades, cancelled, partial] = result;
  for (const auto &id : cancelled)
    CancelOrder(id);

  for (const auto &[id, quantity] : partial)
    m_Orders[id].list_iterator->Fill(quantity);

  // TODO:Successfull trades (Fill or Partial)
}
} // namespace ob::engine
