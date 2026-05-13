#pragma once

#include "Order.hpp"
#include "Trade.hpp"
#include "globals.hpp"

#include <vector>

namespace ob::engine {
struct SnapshotLevel {
  Price price;
  Quantity quantity;
};

struct OrderBookSnapshot {
  std::vector<std::pair<Price, Quantity>> bids, asks;
  std::vector<Trade> trades;
};

struct AuditLevel {
  Price price;
  Quantity quantity;
  std::vector<Order> orders;
};

struct AuditBookSnapshot {
  static void Serialize(std::vector<std::byte> &buffer,
                        const AuditBookSnapshot &snapshot) {}

  std::vector<AuditLevel> bids, asks;
};
} // namespace ob::engine
