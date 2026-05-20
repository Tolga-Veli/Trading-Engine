#pragma once

#include "Logging.hpp"
#include "globals.hpp"

namespace Hermes::core {
class alignas(64) Order {
public:
  Order() = delete;
  Order(OrderID orderID, ClientID clientID, Price price, Quantity quantity,
        Side side, OrderType order_type, TimeInForce tif, MatchType match_type,
        Flags flag) noexcept
      : m_OrderID(orderID), m_ClientID(clientID),
        m_Timestamp(core::GetCurrentTime()), m_Price(price),
        m_IntialQuantity(quantity), m_RemainingQuantity(quantity), m_Side(side),
        m_OrderType(order_type), m_TimeInForce(tif), m_Flags(flag),
        m_MatchType(match_type) {}

  ~Order() noexcept = default;

  Order(const Order &) = default;
  Order(Order &&) = default;
  Order &operator=(const Order &) = default;
  Order &operator=(Order &&) = default;

  OrderID GetOrderID() const noexcept { return m_OrderID; }
  ClientID GetClientID() const noexcept { return m_ClientID; }
  Price GetPrice() const noexcept { return m_Price; }
  Quantity GetInitialQuantity() const noexcept { return m_IntialQuantity; }
  Quantity GetRemainingQuantity() const noexcept { return m_RemainingQuantity; }
  Quantity GetFilledQuantity() const noexcept {
    return m_IntialQuantity - m_RemainingQuantity;
  }
  TimeNs GetTimestamp() const noexcept { return m_Timestamp; }
  Side GetSide() const noexcept { return m_Side; }
  OrderType GetOrderType() const noexcept { return m_OrderType; }
  TimeInForce GetTimeInForce() const noexcept { return m_TimeInForce; }
  Flags GetFlags() const noexcept { return m_Flags; }
  MatchType GetMatchType() const noexcept { return m_MatchType; }

  bool isFilled() const noexcept { return m_RemainingQuantity == 0; }

  void ModifyOrder(Price new_price, Quantity new_quantity) noexcept {
    const Quantity filled = GetFilledQuantity();
    if (filled > new_quantity) {
      HERMES_ERROR(
          "Modified quantity is less than the already filled quantity.");
      return;
    }

    if (new_price != m_Price || new_quantity > m_IntialQuantity)
      m_Timestamp = core::GetCurrentTime();

    m_Price = new_price;
    m_RemainingQuantity = new_quantity - filled;
    m_IntialQuantity = new_quantity;
  }

  void Fill(Quantity quantity) noexcept {
    if (quantity > m_RemainingQuantity) {
      HERMES_ERROR("Order ({}) cannot be filled for more than its capcity.",
                   m_OrderID);
      return;
    }

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
                "Time in Force: {},  Flags:{} \n",
                m_OrderID, m_ClientID, m_Price, m_IntialQuantity,
                m_RemainingQuantity, m_Timestamp, core::to_string(m_Side),
                core::to_string(m_OrderType), core::to_string(m_TimeInForce),
                core::to_string(m_Flags));
  }

private:
  // 8-Byte Fields (48 Bytes Total)
  OrderID m_OrderID;
  ClientID m_ClientID;
  Price m_Price;
  Quantity m_IntialQuantity, m_RemainingQuantity;
  TimeNs m_Timestamp;

  // 2-Byte Field (4 Bytes Total)
  Flags m_Flags;
  ExecutionInstructions m_ExecInst;

  // 1-Byte Fields (5 Bytes Total)
  Side m_Side;
  OrderType m_OrderType;
  TimeInForce m_TimeInForce;
  MatchType m_MatchType;

  // Explicit Zero-Padding (9 Bytes)
  // Pads out the remaining bytes so the total size is 64
  u8 m_Padding[8]{};
};

static_assert(sizeof(Order) == 64, "Order size unexcpected");
static_assert(std::is_trivially_copyable_v<Order>);
} // namespace Hermes::core
