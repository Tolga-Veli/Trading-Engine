#pragma once

#include "Order.hpp"

#include <cstdint>

namespace ob {
namespace engine {
template <class Strategy> class OrderBook;
}

namespace matching {

struct PriceTimePriority {
public:
  void Match(Order &order, engine::OrderBook<PriceTimePriority> &book);

private:
  std::uint64_t m_Counter = 0;
};

} // namespace matching
} // namespace ob
