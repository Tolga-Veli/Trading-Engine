#pragma once

#include <unordered_map>

#include "Order.hpp"
#include "Trade.hpp"

namespace ob::engine {

class OrderBookQuery;

struct MatchResult {
  std::vector<Trade> trades;
  std::vector<OrderID> cancelledOrderIDs;
  std::unordered_map<OrderID, Quantity> partialFills;
};

class IMatchingStrategy {
public:
  virtual ~IMatchingStrategy() = default;
  [[nodiscard]] virtual MatchResult Match(const Order &incoming_order,
                                          const OrderBookQuery &book) const = 0;

  static std::size_t m_Counter;
};

class FIFO_Matching : public IMatchingStrategy {
public:
  [[nodiscard]] MatchResult Match(const Order &incoming_order,
                                  const OrderBookQuery &book) const override;
};
} // namespace ob::engine
