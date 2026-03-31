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
using ClientOrderID = u64;
using TradeID = u64;
using Price = i64; // in 1/10th of a cent therefore  1000 = 1$
using Quantity = i64;
using Time = std::chrono::nanoseconds;

template <typename... Fs> struct Overloaded : Fs... {
  using Fs::operator()...;
};

template <typename... Fs> Overloaded(Fs...) -> Overloaded<Fs...>;

enum class Side { Buy = 0, Sell };

enum class OrderType { Limit = 0, Market, Stop, StopLimit };

enum class TimeInForce {
  GoodTillCancelled = 0,
  DayOrder,
  ImmediateOrCancel,
  FillOrKill,
  FillAndKill
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
  switch (order_type) {
  case OrderType::Limit:
    return "Limit";
  case OrderType::Market:
    return "Market";
  case OrderType::Stop:
    return "Stop";
  case OrderType::StopLimit:
    return "StopLimit";
  }
  return "";
}

inline constexpr std::string_view to_string(TimeInForce tif) {
  switch (tif) {
  case TimeInForce::DayOrder:
    return "DayOrder";
  case TimeInForce::GoodTillCancelled:
    return "GoodTillCancelled";
  case TimeInForce::ImmediateOrCancel:
    return "ImmediateOrCancel";
  case TimeInForce::FillOrKill:
    return "FillOrKill";
  case TimeInForce::FillAndKill:
    return "FillAndKill";
  }
  return "";
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
  switch (matchType) {
  case MatchType::Standard:
    return "Standard";
  case MatchType::Midpoint:
    return "Midpoint";
  case MatchType::HiddenLiquidity:
    return "HiddenLiquidity";
  case MatchType::Auction:
    return "Auction";
  }
  return "";
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
