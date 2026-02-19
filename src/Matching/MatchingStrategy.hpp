#pragma once

#include <cstdint>

namespace ob::engine {

class OrderBook;

class IMatchingStrategy {
public:
  virtual ~IMatchingStrategy() = default;
  virtual void Match(OrderBook &book) = 0;
};

class FIFO_Matching : public IMatchingStrategy {
public:
  void Match(OrderBook &book) override;

  static std::uint64_t m_Counter;
};
} // namespace ob::engine
