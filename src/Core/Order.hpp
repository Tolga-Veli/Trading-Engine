#pragma once

#include "Logging.hpp"
#include "globals.hpp"

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

  Order(const Order &) = default;
  Order(Order &&) = default;
  Order &operator=(const Order &) = default;
  Order &operator=(Order &&) = default;

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
      HERMES_FATAL(
          "Modified quantity is less than the already filled quantity.");

    if (new_price != m_Price || new_quantity > m_IntialQuantity)
      m_Timestamp = core::GetCurrentTime();

    m_Price = new_price;
    m_RemainingQuantity = new_quantity - filled;
    m_IntialQuantity = new_quantity;
  }

  void Fill(Quantity quantity) {
    if (quantity > m_RemainingQuantity)
      HERMES_FATAL("Order ({}) cannot be filled for more than its capcity.",
                   m_OrderID);

    m_RemainingQuantity -= quantity;
  }

  Quantity UpdateQuantity(Quantity new_quantity) {
    Quantity diff = new_quantity - m_RemainingQuantity;
    m_RemainingQuantity = new_quantity;
    return diff;
  }

  void log() const {
    HERMES_INFO("Order ID: {}, ClientID: {}, Price: {}, InitialQuantity: {}, "
                "RemainingQuantity: {}, Time: {}, Side: {}, Order Type: {}, "
                "Time in Force: {}, Flags:{} \n",
                m_OrderID, m_ClientID, m_Price, m_IntialQuantity,
                m_RemainingQuantity, m_Timestamp, core::to_string(m_Side),
                core::to_string(m_OrderType), core::to_string(m_TimeInForce),
                core::to_string(m_Flags));
  }

private:
  OrderID m_OrderID;
  ClientID m_ClientID;
  Price m_Price;
  Quantity m_IntialQuantity, m_RemainingQuantity;
  Time m_Timestamp;
  Side m_Side;
  OrderType m_OrderType;
  TimeInForce m_TimeInForce;
  Flags m_Flags;
};
} // namespace ob
