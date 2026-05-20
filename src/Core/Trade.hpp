#pragma once

#include "Logging.hpp"
#include "globals.hpp"

namespace Hermes::core {

class alignas(8) Trade {
public:
  Trade() = default;
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
  TimeNs GetTimestamp() const noexcept { return m_Timestamp; }

  void log() const {
    HERMES_INFO("TradeID: {}, Maker OrderID: {}, Taker OrderID: {}, Price: {}, "
                "Quantity: {}, Taker Side: {}, Match Type: {}, Timestamp: {}\n",
                m_TradeID, m_MakerOrderID, m_TakerOrderID, m_Price, m_Quantity,
                core::to_string(m_TakerSide), core::to_string(m_MatchType),
                m_Timestamp);
  }

private:
  // 8-Byte Fields (48 Bytes)
  TradeID m_TradeID;
  OrderID m_MakerOrderID;
  OrderID m_TakerOrderID;
  Price m_Price;
  Quantity m_Quantity;
  TimeNs m_Timestamp;

  // 1-Byte Fields (2 Bytes)
  Side m_TakerSide;
  MatchType m_MatchType;

  // Explicit Zero-Padding (6 Bytes)
  // Pads out the remaining bytes so the total size is a multiple of 8
  u8 m_Padding[6]{};
};

static_assert(sizeof(Trade) == 56, "Trade size unecpected");
static_assert(std::is_trivially_copyable_v<Trade>);

} // namespace Hermes::core
