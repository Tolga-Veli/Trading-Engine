#pragma once

#include "Logging.hpp"
#include "globals.hpp"

namespace ob {
class Trade {
public:
  Trade() = delete;
  Trade(TradeID tradeID, OrderID bidOrderID, OrderID askOrderID, Price bidPrice,
        Price askPrice, Quantity quantity, MatchType matchType)
      : m_TradeID(tradeID), m_BidOrderID(bidOrderID), m_AskOrderID(askOrderID),
        m_BidPrice(bidPrice), m_AskPrice(askPrice), m_Quantity(quantity),
        m_MatchType(matchType), m_Timestamp(core::GetCurrentTime()) {}

  const TradeID GetTradeID() const noexcept { return m_TradeID; }
  const OrderID GetBidOrderID() const noexcept { return m_BidOrderID; }
  const OrderID GetAskOrderID() const noexcept { return m_AskOrderID; }
  Time GetTimestamp() const noexcept { return m_Timestamp; }
  Price GetAskPrice() const noexcept { return m_AskPrice; }
  Price GetBidPrice() const noexcept { return m_BidPrice; }
  Quantity GetQuantity() const noexcept { return m_Quantity; }
  MatchType GetMatchType() const noexcept { return m_MatchType; }

  void log() const {
    HERMES_INFO("TradeID: {}, bidOrderID: {}, askOrderID: {}, Timestamp: {}, "
                "askPrice: {}, Quantity: {}, MatchType: {}\n",
                m_TradeID, m_BidOrderID, m_AskOrderID, m_Timestamp, m_BidPrice,
                m_AskPrice, m_Quantity, core::to_string(m_MatchType));
  }

private:
  const TradeID m_TradeID;
  const OrderID m_BidOrderID, m_AskOrderID;
  const Time m_Timestamp;
  const Price m_BidPrice, m_AskPrice;
  const Quantity m_Quantity;
  const MatchType m_MatchType;
};
} // namespace ob
