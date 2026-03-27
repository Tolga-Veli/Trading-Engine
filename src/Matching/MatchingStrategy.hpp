#pragma once

#include <cstdint>

namespace ob::engine {

template <class T> class OrderBook;

class FIFO_Matching {
public:
  void Match(OrderBook<FIFO_Matching> &book);

  inline static std::uint64_t m_Counter = 0;
};
} // namespace ob::engine
