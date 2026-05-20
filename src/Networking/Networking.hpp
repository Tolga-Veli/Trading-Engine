#pragma once

#include "globals.hpp"

#include "Commands.hpp"
#include "Events.hpp"

namespace ob::networking {
using RequestID = std::uint64_t;

struct Request {
  RequestID requestID;
  engine::CommandPayload payload;
};

struct Response {
  RequestID requestID;

  bool ok;
  OrderID orderID;
  engine::ErrorCode error;
};
}; // namespace ob::networking
//
