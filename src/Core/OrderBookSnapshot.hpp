#pragma once

#include "Order.hpp"
#include "Trade.hpp"
#include "globals.hpp"

#include <vector>

namespace Hermes::engine {
struct alignas(8) SnapshotLevel {
  Price price;       // 8 bytes
  Quantity quantity; // 8 bytes
};

static_assert(sizeof(SnapshotLevel) == 16, "SnapshotLevel unexpected size");
static_assert(std::is_trivially_copyable_v<SnapshotLevel>);

struct OrderBookSnapshot {
  std::vector<SnapshotLevel> bids, asks;
  std::vector<core::Trade> trades;
};

struct AuditLevel {
  Price price;
  Quantity quantity;
  std::vector<core::Order> orders;
};

struct AuditBookSnapshot {
  AuditBookSnapshot() = default;

  static void Serialize(std::vector<std::byte> &buffer,
                        const AuditBookSnapshot &snapshot) {}

  std::vector<AuditLevel> bids, asks;
};
} // namespace Hermes::engine
