#pragma once

#include "Trade.hpp"
#include "globals.hpp"

#include <cstring>

namespace Hermes::engine {

enum class EventType : u8 {
  None = 0,
  OrderAccepted,
  OrderRejected,
  ModifyAccepted,
  ModifyRejected,
  CancelAccepted,
  CancelRejected,
  Trade,
};

enum class ErrorCode : u8 {
  None = 0,
  Success,
  InvalidRequest,
  InvalidModify,
  InvalidCancel,
  InsufficientLiquidity,
  PostOnlyViolation,
  InvalidOrderCombination,
  NotImplemented,
  UnsupportedTimeInForce,
  Unauthorized
};

namespace Events {

struct alignas(8) OrderAccepted {
  ClientID clientID; // 8 bytes
  OrderID orderID;   // 8 bytes
};

static_assert(sizeof(OrderAccepted) == 16,
              "Events::OrderAccepted unexpected size");
static_assert(std::is_trivially_copyable_v<OrderAccepted>);

struct alignas(8) OrderRejected {
  ClientID clientID;   // 8 bytes
  ErrorCode errorCode; // 1 byte
  uint8_t pad[7]{};    // 7 bytes explicit padding
};

static_assert(sizeof(OrderRejected) == 16,
              "Events::OrderRejected unexpected size");
static_assert(std::is_trivially_copyable_v<OrderRejected>);

struct alignas(8) ModifyAccepted {
  ClientID clientID;   // 8 bytes
  OrderID old_orderID; // 8 bytes
  OrderID new_orderID; // 8 bytes
};

static_assert(sizeof(ModifyAccepted) == 24,
              "Events::ModifyAccepted unexpected size");
static_assert(std::is_trivially_copyable_v<ModifyAccepted>);

struct alignas(8) ModifyRejected {
  ClientID clientID;   // 8 bytes
  OrderID orderID;     // 8 bytes
  ErrorCode errorCode; // 1 byte
  uint8_t pad[7]{};    // 7 bytes explicit padding
};

static_assert(sizeof(ModifyRejected) == 24,
              "Events::ModifyRejected unexpected size");
static_assert(std::is_trivially_copyable_v<ModifyRejected>);

struct alignas(8) CancelAccepted {
  ClientID clientID; // 8 bytes
  OrderID orderID;   // 8 bytes
};

static_assert(sizeof(CancelAccepted) == 16,
              "Events::CancelAccepted unexpected size");
static_assert(std::is_trivially_copyable_v<CancelAccepted>);

struct alignas(8) CancelRejected {
  ClientID clientID;   // 8 bytes
  OrderID orderID;     // 8 bytes
  ErrorCode errorCode; // 1 byte
  uint8_t pad[7]{};    // 7 bytes explicit padding
};

static_assert(sizeof(CancelRejected) == 24,
              "Events::CancelRejected unexpected size");
static_assert(std::is_trivially_copyable_v<CancelRejected>);
} // namespace Events

/*
 * Event - tagged union
 * Invariant: m_EventType always reflects which union is active
 * Enforcement: Construction only through make factories
 *
 *
 */
class alignas(8) EventPayload {
public:
  EventPayload() : m_EventType(EventType::None), m_Data{} {}

  [[nodiscard]] static EventPayload
  MakeOrderAccepted(ClientID clientID, OrderID orderID) noexcept {
    EventPayload event;
    event.m_EventType = EventType::OrderAccepted;
    event.m_Data.order_acc = {clientID, orderID};
    return event;
  }

  [[nodiscard]] static EventPayload
  MakeOrderRejected(ClientID clientID, ErrorCode error_code) noexcept {
    EventPayload event;
    event.m_EventType = EventType::OrderRejected;
    event.m_Data.order_rej = {clientID, error_code};
    return event;
  }

  [[nodiscard]] static EventPayload
  MakeModifyAccepted(ClientID cid, OrderID old_oid, OrderID new_oid) noexcept {
    EventPayload event;
    event.m_EventType = EventType::ModifyAccepted;
    event.m_Data.mod_acc = {cid, old_oid, new_oid};
    return event;
  }

  [[nodiscard]] static EventPayload
  MakeModifyRejected(ClientID cid, OrderID oid, ErrorCode ec) noexcept {
    EventPayload event;
    event.m_EventType = EventType::ModifyRejected;
    event.m_Data.mod_rej = {cid, oid, ec};
    return event;
  }

  [[nodiscard]] static EventPayload MakeCancelAccepted(ClientID cid,
                                                       OrderID oid) noexcept {
    EventPayload event;
    event.m_EventType = EventType::CancelAccepted;
    event.m_Data.cancel_acc = {cid, oid};
    return event;
  }

  [[nodiscard]] static EventPayload
  MakeCancelRejected(ClientID cid, OrderID oid, ErrorCode ec) noexcept {
    EventPayload event;
    event.m_EventType = EventType::CancelRejected;
    event.m_Data.cancel_rej = {cid, oid, ec};
    return event;
  }

  [[nodiscard]] static EventPayload MakeTrade(core::Trade t) noexcept {
    EventPayload event;
    event.m_EventType = EventType::Trade;
    event.m_Data.trade = t;
    return event;
  }

  [[nodiscard]] EventType GetType() const noexcept { return m_EventType; }

  [[nodiscard]] bool IsNone() const noexcept {
    return m_EventType == EventType::None;
  }

  static void Serialize(std::byte *buffer, const EventPayload &event) noexcept {
    std::memcpy(buffer, &event, sizeof(EventPayload));
  }

  static EventPayload Deserialize(const std::byte *src) noexcept {
    EventPayload event;
    std::memcpy(&event, src, sizeof(EventPayload));
    return event;
  }

private:
  // Takes the size of its biggest field, which is trade => 56 bytes
  union {
    Events::OrderAccepted order_acc;
    Events::OrderRejected order_rej;
    Events::ModifyAccepted mod_acc;
    Events::ModifyRejected mod_rej;
    Events::CancelAccepted cancel_acc;
    Events::CancelRejected cancel_rej;
    core::Trade trade;
  } m_Data;

  // 1 bytes
  EventType m_EventType;

  u8 m_Padding[7]{};
};

static_assert(sizeof(EventPayload) == 64, "EventPayload unexpected size");
static_assert(
    std::is_trivially_copyable_v<EventPayload>,
    "EventPayload must be trivially copyable for fast serialization!");

} // namespace Hermes::engine
