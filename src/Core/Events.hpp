#pragma once

#include "OrderBookSnapshot.hpp"
#include "globals.hpp"

#include <variant>

namespace ob::engine {

enum class ErrorCode { None = 0, InvalidRequest = 1 };

namespace EventTypes {
struct OrderAccepted {
  ClientID clientID;
  ClientOrderID clientOrderID;
  OrderID orderID;
};

struct OrderRejected {
  ClientID clientID;
  ClientOrderID clientOrderID;
  ErrorCode errorCode;
};

struct ModifyAccepted {
  ClientID clientID;
  ClientOrderID clientOrderID;
  OrderID orderID;
};

struct ModifyRejected {
  ClientID clientID;
  ClientOrderID clientOrderID;
  ErrorCode errorCode;
};

struct CancelAccepted {
  ClientID clientID;
  ClientOrderID clientOrderID;
  OrderID orderID;
};

struct CancelRejected {
  ClientID clientID;
  ClientOrderID clientOrderID;
  ErrorCode errorCode;
};

struct SnapshotRequestAccepted {
  OrderBookSnapshot snapshot;
};

struct SnapshotRequestRejected {
  ErrorCode errorCode;
};

} // namespace EventTypes

using Event = std::variant<
    std::monostate, EventTypes::OrderAccepted, EventTypes::OrderRejected,
    EventTypes::ModifyAccepted, EventTypes::ModifyRejected,
    EventTypes::CancelAccepted, EventTypes::CancelRejected,
    EventTypes::SnapshotRequestAccepted, EventTypes::SnapshotRequestRejected>;

} // namespace ob::engine
