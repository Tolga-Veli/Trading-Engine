#pragma once

#include "Trade.hpp"
#include "globals.hpp"
#include <vector>

namespace ob::engine {
struct OrderBookSnapshot {
  std::vector<std::pair<Price, Quantity>> bids, asks;
  std::vector<Trade> trades;
};
} // namespace ob::engine
