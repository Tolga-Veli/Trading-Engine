#pragma once

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace Hermes::core {
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using ClientID = u64;
using OrderID = u64;
using Price = i64; // in 1/10th of a cent therefore  1000 = 1$
using Quantity = u64;

using TimeNs = i64;

using AccountID = u64;
using ClientOrderID = u64;
using TradeID = u64;
using MatchID = u64;

enum class Side : u8 { Buy = 0, Sell = 1 };

enum class OrderType : u8 {
  Limit = 0,
  Market = 1,
  Complex = 2,
  Stop,
  StopLimit,
  MarketIfTouched,
  LimitIfTouched,
  Pegged
};

enum class TimeInForce : u8 {
  GTC = 0, // Good 'Till Cancelled - Supported
  Day, // Day order - expires automatically at the end of the trading session
       // Not supported

  IOC, // Immediate-Or-Cancel (fills what it can immediately, cancels rest) -
       // Supported

  FOK, // Fill-Or-Kill (fills entire quantity immediately, or cancels
       // everything) -  Supported

  GTD, // Good 'Till Date/Time (requires ExpiryTime field) - Not supported
  ATO, // At the Opening (Auction only) -  Not supported
  ATC, // At the Close (Auction only) - Not supported
};

enum class PegScope : u8 {
  None = 0,
  Primary = 1,
  Market = 2,
  Midpoint = 3,
};

enum class MatchType : u8 {
  Standard,
  Midpoint,        // Not supported
  HiddenLiquidity, // Not supported
  Auction,         // Not supported
};

enum class ExecutionInstructions : u16 {
  None = 0,
  ParticipateDoNotInitiate = 1,
  AllOrNone = 1 << 1,
  ReduceOnly = 1 << 2,
  IntermarketSwap = 1 << 3,
};

enum class Flags : u16 {
  None = 0,
  Hidden = 1,
  Iceberg = 1 << 1,
  PostOnly = 1 << 2,
};

inline Flags operator|(Flags a, Flags b) {
  return static_cast<Flags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline Flags operator&(Flags a, Flags b) {
  return static_cast<Flags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline Flags &operator|=(Flags &a, Flags b) {
  a = a | b;
  return a;
}

inline bool HasFlag(Flags value, Flags flag) {
  return (static_cast<u16>(value) & static_cast<u16>(flag)) != 0;
}

inline std::string format_price(Price price) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2)
     << (static_cast<double>(price) / 100.0);
  return ss.str();
}

inline std::string format_quantity(Quantity value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

inline constexpr std::string_view to_string(Side side) {
  if (side == Side::Buy)
    return "Buy";
  else
    return "Sell";
}

inline constexpr std::string_view to_string(OrderType order_type) {
  using enum OrderType;
  switch (order_type) {
  case Limit:
    return "Limit";
  case Market:
    return "Market";
  case Stop:
    return "Stop";
  case StopLimit:
    return "StopLimit";
  default:
    return "";
  }
}

inline constexpr std::string_view to_string(TimeInForce tif) {
  using enum TimeInForce;
  switch (tif) {
  case Day:
    return "DayOrder";
  case GTC:
    return "GoodTillCancelled";
  case IOC:
    return "ImmediateOrCancel";
  case FOK:
    return "FillOrKill";
  case GTD:
    return "GoodTillDate";
  default:
    return "";
  }
}

inline std::string to_string(Flags flag) {
  if (flag == Flags::None)
    return "None";

  std::string str = "";
  if ((flag & Flags::Hidden) == Flags::Hidden)
    str += "Hidden|";
  if ((flag & Flags::Iceberg) == Flags::Iceberg)
    str += "Iceberg|";
  if ((flag & Flags::PostOnly) == Flags::PostOnly)
    str += "PostOnly|";

  if (!str.empty())
    str.pop_back();

  return str;
}

inline constexpr std::string_view to_string(MatchType matchType) {
  using enum MatchType;
  switch (matchType) {
  case Standard:
    return "Standard";
  case Midpoint:
    return "Midpoint";
  case HiddenLiquidity:
    return "HiddenLiquidity";
  case Auction:
    return "Auction";
  default:
    return "";
  }
}

inline static TimeNs GetCurrentTime() {
  return std::chrono::steady_clock::now().time_since_epoch().count();
}
} // namespace Hermes::core

namespace Hermes {
using i32 = core::i32;
using i64 = core::i64;

using u8 = core::u8;
using u16 = core::u16;
using u32 = core::u32;
using u64 = core::u64;

using core::ClientID;
using core::OrderID;
using core::Price;
using core::Quantity;
using core::TradeID;

using core::Flags;
using core::MatchType;
using core::OrderType;
using core::Side;
using core::TimeInForce;
using core::TimeNs;

} // namespace Hermes
