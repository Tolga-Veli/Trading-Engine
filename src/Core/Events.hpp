#pragma once

#include <variant>

#include "globals.hpp"

namespace ob::engine {

enum class EventType {
  None = 0,
  OrderAccepted = 1,
  OrderExpired,
  OrderRejected,
  ModifyAccepted,
  ModifyRejected,
  CancelAccepted,
  CancelRejected,
};

enum class ErrorCode {
  Success = 0,
  InvalidRequest = 1,
  InvalidModify,
  InvalidCancel,
  InsufficientLiquidity,
  PostOnlyViolation
};

namespace EventTypes {
struct OrderAccepted {
  ClientID clientID;
  OrderID orderID;
};

struct OrderExpired {
  ClientID clientID;
  OrderID orderID;
  Quantity remaining_quantity;
};

struct OrderRejected {
  ClientID clientID;
  ErrorCode errorCode;
};

struct ModifyAccepted {
  ClientID clientID;
  OrderID old_orderID;
  OrderID new_orderID;
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

class Event {
public:
  using EventVariant =
      std::variant<std::monostate, EventTypes::OrderAccepted,
                   EventTypes::OrderExpired, EventTypes::OrderRejected,
                   EventTypes::ModifyAccepted, EventTypes::ModifyRejected,
                   EventTypes::CancelAccepted, EventTypes::CancelRejected>;

  Event() = default;
  Event(const EventVariant &v) : m_Variant(v) {}

  constexpr EventType GetType() const {
    return std::visit(
        [](auto &&arg) -> EventType {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::monostate>)
            return EventType::None;
          else if constexpr (std::is_same_v<T, std::monostate>)
            return EventType::OrderAccepted;
          else if constexpr (std::is_same_v<T, EventTypes::OrderAccepted>)
            return EventType::OrderExpired;
          else if constexpr (std::is_same_v<T, EventTypes::OrderExpired>)
            return EventType::OrderRejected;
          else if constexpr (std::is_same_v<T, EventTypes::OrderRejected>)
            return EventType::OrderRejected;
          else if constexpr (std::is_same_v<T, EventTypes::ModifyAccepted>)
            return EventType::ModifyAccepted;
          else if constexpr (std::is_same_v<T, EventTypes::ModifyRejected>)
            return EventType::ModifyRejected;
          else if constexpr (std::is_same_v<T, EventTypes::CancelAccepted>)
            return EventType::CancelAccepted;
          else if constexpr (std::is_same_v<T, EventTypes::CancelRejected>)
            return EventType::CancelRejected;
        },
        m_Variant);
  }

  template <typename... Fs> void Decompose(Fs &&...fs) const {
    std::visit(Overloaded{std::forward<Fs>(fs)...}, m_Variant);
  }

  static void Serialize(std::vector<std::byte> &buffer, const Event &cmd) {}

private:
  EventVariant m_Variant;
};

} // namespace ob::engine
