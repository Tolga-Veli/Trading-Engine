#pragma once

#include "Core/Commands.hpp"
#include "Core/Events.hpp"
#include "Core/Order.hpp"
#include "Core/OrderBookSnapshot.hpp"
#include "Matching/Matching.hpp"

#include <array>
#include <list>
#include <map>
#include <memory_resource>
#include <unordered_map>
#include <variant>

namespace ob::engine {
inline constexpr u32 MaxEventsPerCommand = 128;
inline thread_local std::array<Event, MaxEventsPerCommand> t_EventScratch;

// use alignas(64) so LevelData is not a cache boundary and cause an additional
// fetch on every best-bid/ask comparison
struct alignas(64) LevelData {
  Quantity volume{0};
  std::pmr::list<Order> orders;

  LevelData() = delete;
  explicit LevelData(std::pmr::memory_resource *mr) : orders(mr) {}

  ~LevelData() = default;
};

struct OrderEntry {
  std::pmr::list<Order>::iterator list_it;
  std::pmr::map<Price, LevelData>::iterator level_it;
};

enum class MarketState : u8 {
  Closed = 0,
  PreOpen = 1,
  Auction = 2,
  Continuous = 3
};

template <class MatchingStrategy> class OrderBook {
  static_assert(
      requires(MatchingStrategy s, Order &order, OrderBook &book) {
        s.Match(order, book);
      }, "MatchingStrategy::Match must exist and accept an Order& and an"
         "OrderBook<S>&");

public:
  // Pool default: 32 MiB
  static constexpr u64 DefaultPoolSize = 32 * 1024 * 1024;

  explicit OrderBook(u64 pool_size = DefaultPoolSize) noexcept
      : m_Buffer(pool_size), m_Pool(&m_Buffer), m_Bids(&m_Pool),
        m_Asks(&m_Pool), m_Orders(&m_Pool), m_ExpiryWatchlist(&m_Pool) {}

  ~OrderBook() = default;

  // Non-copyable, non-movable
  // moving the PMR pool resource would silently invalidate everything
  OrderBook(const OrderBook &) = delete;
  OrderBook &operator=(const OrderBook &) = delete;
  OrderBook(OrderBook &&) = delete;
  OrderBook &operator=(OrderBook &&) = delete;

  [[nodiscard]] const Order *FindOrder(const OrderID &orderID) const noexcept;

  Quantity GetBidVolumeAtPrice(Price price) const noexcept;
  Quantity GetAskVolumeAtPrice(Price price) const noexcept;

  [[nodiscard]] MarketState GetMarketState() const noexcept {
    return m_MarketState;
  }

  // Returns nullptr when the side is empty - callers must check
  [[nodiscard]] Order *GetBestBid() noexcept;
  [[nodiscard]] Order *GetBestAsk() noexcept;

  [[nodiscard]] OrderBookSnapshot GetSnapshot(uint32_t depth) const noexcept;

  // called by the MatchingStrategy on every partial or full match
  void RecordFill(OrderID makerOrderID, OrderID takerOrderID, Price price,
                  Quantity quantity, Side takerSide,
                  MatchType match_type) noexcept;

  [[nodiscard]] std::span<const Event> Apply(const Command &cmd) noexcept {
    m_ScratchCounter = 0;
    cmd.Decompose([this](auto &&arg) noexcept { Handle(arg); });
    return {t_EventScratch.data(), m_ScratchCounter};
  }

private:
  std::pmr::monotonic_buffer_resource m_Buffer;
  std::pmr::unsynchronized_pool_resource m_Pool;

  std::pmr::map<Price, LevelData, std::greater<Price>> m_Bids;
  std::pmr::map<Price, LevelData, std::less<Price>> m_Asks;
  std::pmr::unordered_map<OrderID, OrderEntry> m_Orders;
  std::pmr::multimap<Time, OrderID> m_ExpiryWatchlist; // TODO: prune GTD orders

  MarketState m_MarketState{MarketState::Closed};
  MatchingStrategy m_MatchingStrategy{};

  u32 m_ScratchCounter{0};
  OrderID m_OrderCounter{1};
  TradeID m_TradeCounter{1};

  void Handle(const std::monostate &) const noexcept {}
  void Handle(const CommandTypes::AddOrder &cmd) noexcept;
  void Handle(const CommandTypes::ModifyOrder &cmd) noexcept;
  void Handle(const CommandTypes::CancelOrder &cmd) noexcept;

  [[nodiscard]] ErrorCode
  InternalAddOrder(ClientID clientID, Price price, Quantity quantity, Side side,
                   OrderType order_type, TimeInForce tif, MatchType match_type,
                   Flags flags) noexcept;

  [[nodiscard]] ErrorCode InternalModifyOrder(ClientID clientID,
                                              OrderID orderID, Price new_price,
                                              Quantity new_quantity) noexcept;

  [[nodiscard]] ErrorCode InternalCancelOrder(ClientID clientID,
                                              OrderID orderID) noexcept;

  friend class matching::PriceTimePriority;
};
} // namespace ob::engine

#include "OrderBook.inl"
