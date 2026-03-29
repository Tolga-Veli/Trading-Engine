#pragma once

#include "globals.hpp"

#include <variant>

namespace ob::engine {

enum class ErrorCode { None = 0, InvalidRequest = 1 };

namespace EventTypes {
struct OrderAccepted {
  ClientID clientID;
  OrderID orderID;
};

struct OrderRejected {
  ClientID clientID;
  ErrorCode errorCode;
};

struct ModifyAccepted {
  ClientID clientID;
  OrderID orderID;
};

struct ModifyRejected {
  ClientID clientID;
  OrderID orderID;
  ErrorCode errorCode;
};

struct CancelAccepted {
  ClientID clientID;
  OrderID orderID;
};

struct CancelRejected {
  ClientID clientID;
  OrderID orderID;
  ErrorCode errorCode;
};

} // namespace EventTypes

using Event =
    std::variant<std::monostate, EventTypes::OrderAccepted,
                 EventTypes::OrderRejected, EventTypes::ModifyAccepted,
                 EventTypes::ModifyRejected, EventTypes::CancelAccepted,
                 EventTypes::CancelRejected>;

} // namespace ob::engine
