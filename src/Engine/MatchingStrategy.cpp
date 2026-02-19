#include "MatchingStrategy.hpp"
#include "OrderbookQuery.hpp"

namespace ob::engine {
static std::size_t m_Counter = 0;

[[nodiscard]] MatchResult
FIFO_Matching::Match(const Order &incoming_order,
                     const OrderBookQuery &book) const {
  MatchResult result;
  while (true) {
    if (!book.HasOrders())
      break;

    auto bid = book.GetBestBid(), ask = book.GetBestAsk();
    if (!bid || !ask)
      break;

    auto bidPrice = bid->GetPrice(), askPrice = ask->GetPrice();
    if (bidPrice < askPrice)
      break;

    Quantity quantity =
        std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

    result.partialFills[bid->GetOrderID()] += quantity;
    result.partialFills[ask->GetOrderID()] += quantity;

    if (bid->isFilled())
      result.cancelledOrderIDs.push_back(bid->GetOrderID());
    if (ask->isFilled())
      result.cancelledOrderIDs.push_back(ask->GetOrderID());

    result.trades.emplace_back(std::move(Trade{
        m_Counter++, bid->GetOrderID(), ask->GetOrderID(), bid->GetPrice(),
        ask->GetPrice(), quantity, MatchType::Standard}));
  }

  return result;
}
} // namespace ob::engine
