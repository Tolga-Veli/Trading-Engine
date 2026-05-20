#pragma once

#include "globals.hpp"

#include "Commands.hpp"
#include "Events.hpp"

namespace Hermes::network {
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
}; // namespace Hermes::network
//
