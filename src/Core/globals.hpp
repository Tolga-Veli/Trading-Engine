#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace ob {
using i32 = std::int16_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using ClientID = u64;
using OrderID = u64;
using TradeID = u64;
using Price = i64; // in 1/10th of a cent therefore  1000 = 1$
using Quantity = u64;
using Time = std::chrono::nanoseconds;

template <typename... Fs> struct Overloaded : Fs... {
  using Fs::operator()...;
};

template <typename... Fs> Overloaded(Fs...) -> Overloaded<Fs...>;

enum class Side { Buy = 0, Sell };

enum class OrderType { Limit = 0, Market, Stop, StopLimit };

enum class TimeInForce {
  GoodTillCancelled = 0, // Supported
  DayOrder,              // Not supported
  ImmediateOrCancel,     // Supported
  FillOrKill,            // Supported
  FillAndKill,           // Supported
  GoodTillDate,          // Not supported
  AtTheOpening,          // Not supported
};

enum class MatchType {
  Standard,
  Midpoint,
  HiddenLiquidity,
  Auction,
};

enum class Flags : u8 {
  None = 0,
  Hidden = 1,
  Iceberg = 1 << 1,
  PostOnly = 1 << 2
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

namespace core {
inline std::string format_price(Price price) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2)
     << (static_cast<double>(price) / 100.0);
  return ss.str();
}

inline std::string format_quantity(ob::Quantity value) {
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
  case DayOrder:
    return "DayOrder";
  case GoodTillCancelled:
    return "GoodTillCancelled";
  case ImmediateOrCancel:
    return "ImmediateOrCancel";
  case FillOrKill:
    return "FillOrKill";
  case FillAndKill:
    return "FillAndKill";
  case GoodTillDate:
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

inline static Time GetCurrentTime() {
  return std::chrono::duration_cast<Time>(
      std::chrono::steady_clock::now().time_since_epoch());
}

inline std::string ReadFile(const std::filesystem::path &path) {
  std::ifstream in(path, std::ios::in | std::ios::binary | std::ios::ate);

  if (!in)
    return "";

  auto size = in.tellg();
  if (size <= 0)
    return "";

  std::string result;
  result.resize(size);

  in.seekg(0, std::ios::beg);
  if (!in.read(result.data(), size))
    return "";

  return result;
}

inline bool WriteToFile(const std::vector<std::byte> &buffer,
                        const std::filesystem::path &path) {
  std::ofstream out(path, std::ios::binary);

  if (!out)
    return false;

  out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
  return out.good();
}
} // namespace core
} // namespace ob
