#pragma once

#include "Order.hpp"

#include <cstdint>

namespace Hermes {
namespace engine {

template <class Strategy> class OrderBook;

} // namespace engine

namespace matching {

struct PriceTimePriority {
public:
  void Match(core::Order &order, engine::OrderBook<PriceTimePriority> &book);

private:
  std::uint64_t m_Counter = 0;
};

} // namespace matching
} // namespace Hermes
