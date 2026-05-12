#include "MatchingStrategy.hpp"
#include "Engine/OrderBook.hpp"

namespace ob::matching {
void PriceTimePriority::Match(Order &order,
                              engine::OrderBook<PriceTimePriority> &book) {
  const Side side = order.GetSide();

  if (side == Side::Buy) {
    while (order.GetRemainingQuantity() > 0) {
      Order *top = book.GetBestAsk();

      if (!top)
        break;

      if (order.GetOrderType() != OrderType::Market &&
          order.GetPrice() < top->GetPrice())
        break;

      Quantity quantity =
          std::min(order.GetRemainingQuantity(), top->GetRemainingQuantity());

      order.Fill(quantity);
      top->Fill(quantity);

      book.RecordFill(order.GetClientID(), top->GetClientID(), order.GetPrice(),
                      quantity, top->GetSide(), order.GetMatchType());

      if (top->isFilled()) {
        const auto code =
            book.InternalCancelOrder(top->GetClientID(), top->GetOrderID());
        // TODO: Handle error
      }
    }
  } else {
    while (order.GetRemainingQuantity() > 0) {
      Order *top = book.GetBestBid();

      if (!top)
        break;

      if (order.GetOrderType() != OrderType::Market &&
          order.GetPrice() > top->GetPrice())
        break;

      Quantity quantity =
          std::min(order.GetRemainingQuantity(), top->GetRemainingQuantity());

      order.Fill(quantity);
      top->Fill(quantity);

      book.RecordFill(order.GetClientID(), top->GetClientID(), order.GetPrice(),
                      quantity, top->GetSide(), order.GetMatchType());

      if (top->isFilled()) {
        const auto code =
            book.InternalCancelOrder(top->GetClientID(), top->GetOrderID());
        // TODO: Handle error
      }
    }
  }
}
} // namespace ob::matching
