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

      const Price exec_price = top->GetPrice();
      book.RecordFill(order.GetClientID(), top->GetClientID(), exec_price,
                      quantity, top->GetSide(), order.GetMatchType());

      if (top->isFilled()) {
        const auto code =
            book.InternalCancelOrder(top->GetClientID(), top->GetOrderID());

        top = nullptr;
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

      const Price exec_price = top->GetPrice();
      book.RecordFill(order.GetClientID(), top->GetClientID(), exec_price,
                      quantity, top->GetSide(), order.GetMatchType());

      if (top->isFilled()) {
        const auto code =
            book.InternalCancelOrder(top->GetClientID(), top->GetOrderID());

        top = nullptr;
        // TODO: Handle error
      }
    }
  }
}
} // namespace ob::matching
