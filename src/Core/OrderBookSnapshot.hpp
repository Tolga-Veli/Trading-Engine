#pragma once

#include "Order.hpp"
#include "Trade.hpp"
#include "globals.hpp"

#include <vector>

namespace ob::engine {
struct alignas(8) SnapshotLevel {
  Price price;       // 8 bytes
  Quantity quantity; // 8 bytes
};

static_assert(sizeof(SnapshotLevel) == 16, "SnapshotLevel unexpected size");
static_assert(std::is_trivially_copyable_v<SnapshotLevel>);

struct OrderBookSnapshot {
  std::vector<SnapshotLevel> bids, asks;
  std::vector<Trade> trades;
};

struct AuditLevel {
  Price price;
  Quantity quantity;
  std::vector<Order> orders;
};

struct AuditBookSnapshot {
  AuditBookSnapshot() = default;

  static void Serialize(std::vector<std::byte> &buffer,
                        const AuditBookSnapshot &snapshot) {}

  std::vector<AuditLevel> bids, asks;
};
} // namespace ob::engine
