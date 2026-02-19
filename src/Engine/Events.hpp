#pragma once

#include "ThreadSafeQueue.hpp"
#include "globals.hpp"

#include <variant>

namespace ob::engine {
enum class EventType {
  None = 0,
  OrderAccepted = 1,
  OrderRejected,
  ModifyAccepted,
  ModifyRejected,
  CancelAccepted,
  CancelRejected
};

enum class ErrorCode { None = 0, InvalidRequest = 1 };

namespace Events {
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
} // namespace Events

using Event =
    std::variant<std::monostate, Events::OrderAccepted, Events::OrderRejected,
                 Events::ModifyAccepted, Events::ModifyRejected,
                 Events::CancelAccepted, Events::CancelRejected>;

class EventHandler {
public:
  EventHandler();
  ~EventHandler();

  template <class T> void push(T &&event) {
    m_Queue.push(std::forward<T>(event));
  }

private:
  data::ThreadSafeQueue<Event> m_Queue;
};
} // namespace ob::engine
