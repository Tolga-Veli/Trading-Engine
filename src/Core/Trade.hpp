#pragma once

#include "Logging.hpp"
#include "globals.hpp"

namespace ob {
struct Trade {
  Trade() = delete;
  Trade(TradeID tradeID, OrderID makerOrderID, OrderID takerOrderID,
        Price price, Quantity quantity, Side takerSide, MatchType matchType)
      : m_TradeID(tradeID), m_MakerOrderID(makerOrderID),
        m_TakerOrderID(takerOrderID), m_Price(price), m_Quantity(quantity),
        m_TakerSide(takerSide), m_MatchType(matchType) {}

  ~Trade() noexcept = default;

  Trade(const Trade &) = default;
  Trade &operator=(const Trade &) = default;
  Trade(Trade &&) = default;
  Trade &operator=(Trade &&) = default;

  TradeID GetTradeID() const noexcept { return m_TradeID; }
  OrderID GetMakerOrderID() const noexcept { return m_MakerOrderID; }
  OrderID GetTakerOrderID() const noexcept { return m_TakerOrderID; }
  Price GetPrice() const noexcept { return m_Price; };
  Quantity GetQuantity() const noexcept { return m_Quantity; }
  Side GetTakerSide() const noexcept { return m_TakerSide; }
  MatchType GetMatchType() const noexcept { return m_MatchType; }
  Time GetTimestamp() const noexcept { return m_Timestamp; }

  void log() const {
    HERMES_INFO("TradeID: {}, Maker OrderID: {}, Taker OrderID: {}, Price: {}, "
                "Quantity: {}, Taker Side: {}, Match Type: {}, Timestamp: {}\n",
                m_TradeID, m_MakerOrderID, m_TakerOrderID, m_Price, m_Quantity,
                core::to_string(m_TakerSide), core::to_string(m_MatchType),
                m_Timestamp);
  }

private:
  TradeID m_TradeID;
  OrderID m_MakerOrderID;
  OrderID m_TakerOrderID;
  Price m_Price; // execution price — always the maker's resting price
  Quantity m_Quantity;
  Side m_TakerSide; // which side was the aggressor
  MatchType m_MatchType;
  Time m_Timestamp;
};
} // namespace ob
