#include "MatchingStrategy.hpp"
#include "Engine/OrderBook.hpp"

namespace ob::engine {
void FIFO_Matching::Match(OrderBook<FIFO_Matching> &book) {
  while (true) {
    Order *bid = book.GetBestBid(), *ask = book.GetBestAsk();

    if (!bid || !ask || bid->GetPrice() < ask->GetPrice())
      break;

    Quantity tradeQty =
        std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

    const OrderID bidID = bid->GetOrderID(), askID = ask->GetOrderID();

    bid->Fill(tradeQty);
    ask->Fill(tradeQty);

    /*book.EmitTrade(Trade{m_Counter++, bidID, askID, bid->GetPrice(),
                         ask->GetPrice(), tradeQty, MatchType::Standard});*/

    if (bid->isFilled())
      book.CancelOrder(bidID);

    if (ask->isFilled())
      book.CancelOrder(askID);
  }
}
} // namespace ob::engine
