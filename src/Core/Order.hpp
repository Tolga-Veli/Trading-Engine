#pragma once

#include "globals.hpp"
#include <iostream>

namespace ob {
class Order {
public:
  Order() = delete;
  Order(OrderID orderID, ClientID clientID, Price price, Quantity quantity,
        Side side, OrderType order_type, TimeInForce tif, Flags flag) noexcept
      : m_OrderID(orderID), m_ClientID(clientID),
        m_Timestamp(core::GetCurrentTime()), m_Price(price),
        m_IntialQuantity(quantity), m_RemainingQuantity(quantity), m_Side(side),
        m_OrderType(order_type), m_TimeInForce(tif), m_Flags(flag) {}

  ~Order() noexcept = default;

  const OrderID &GetOrderID() const noexcept { return m_OrderID; }
  const ClientID &GetClientID() const noexcept { return m_ClientID; }
  Price GetPrice() const noexcept { return m_Price; }
  Quantity GetInitialQuantity() const noexcept { return m_IntialQuantity; }
  Quantity GetRemainingQuantity() const noexcept { return m_RemainingQuantity; }
  Quantity GetFilledQuantity() const noexcept {
    return m_IntialQuantity - m_RemainingQuantity;
  }
  Time GetTime() const noexcept { return m_Timestamp; }
  Side GetSide() const noexcept { return m_Side; }
  OrderType GetOrderType() const noexcept { return m_OrderType; }
  TimeInForce GetTimeInForce() const noexcept { return m_TimeInForce; }
  Flags GetFlags() const noexcept { return m_Flags; }
  bool isFilled() const noexcept { return m_RemainingQuantity == 0; }

  void ModifyOrder(Price new_price, Quantity new_quantity) {
    const Quantity filled = GetFilledQuantity();
    if (filled > new_quantity)
      throw std::logic_error(
          "Modified quantity is less than the already filled quantity.");

    if (new_price != m_Price || new_quantity > m_IntialQuantity)
      m_Timestamp = core::GetCurrentTime();

    m_Price = new_price;
    m_RemainingQuantity = new_quantity - filled;
    m_IntialQuantity = new_quantity;
  }

  void Fill(Quantity quantity) {
    if (quantity > m_RemainingQuantity)
      throw std::logic_error(std::format(
          "Order ({}) cannot be filled for more than its capcity.", m_OrderID));

    m_RemainingQuantity -= quantity;
  }

  void info() const {
    std::cout << "OrderID: " << m_OrderID << " , ClientID: " << m_ClientID
              << " , Price: " << m_Price
              << " , Initial Quantity: " << m_IntialQuantity
              << " , Remaining Quantity: " << m_RemainingQuantity
              << " , Time: " << m_Timestamp
              << " , Side: " << core::to_string(m_Side)
              << " , Order Type: " << core::to_string(m_OrderType)
              << " , Time in Force: " << core::to_string(m_TimeInForce)
              << " , Flags: " << core::to_string(m_Flags) << '\n';
  }

private:
  const OrderID m_OrderID;
  const ClientID m_ClientID;
  Price m_Price;
  Quantity m_IntialQuantity, m_RemainingQuantity;
  Time m_Timestamp;
  const Side m_Side;
  const OrderType m_OrderType;
  const TimeInForce m_TimeInForce;
  const Flags m_Flags;
};
} // namespace ob
