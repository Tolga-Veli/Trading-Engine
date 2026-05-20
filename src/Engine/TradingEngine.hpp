#pragma once

#include "Data-Structures/SPSC-Queue.hpp"
#include "Data-Structures/ThreadSafeQueue.hpp"
#include "OrderBook.hpp"

#include <memory>
#include <thread>

namespace ob::engine {
template <class MatchingStrategy, u64 TickRateNs, u64 CommandCapacity,
          u64 EventCapacity>
class alignas(64) TradingEngine {
public:
  using Book = OrderBook<MatchingStrategy>;

  explicit TradingEngine() noexcept {
    m_Snapshots[0] = std::make_shared<OrderBookSnapshot>();
    m_Snapshots[1] = std::make_shared<OrderBookSnapshot>();
    m_ActiveSnapshot.store(m_Snapshots[0].get(), std::memory_order_relaxed);
  }

  TradingEngine(const TradingEngine &) = delete;
  TradingEngine &operator=(const TradingEngine &) = delete;
  TradingEngine(TradingEngine &&) = delete;
  TradingEngine &operator=(TradingEngine &&) = delete;

  ~TradingEngine() { Stop(); }

  void Start() {
    m_Thread = std::jthread([this](std::stop_token st) { Run(st); });
  };

  void Stop() { m_Thread.request_stop(); }

  void AddOrder(ClientID clientID, Price price, Quantity quantity, Side side,
                OrderType order_type, TimeInForce tif, MatchType match_type,
                Flags flags) {
    m_CommandQueue.push(CommandPayload::MakeAdd(
        clientID, price, quantity, side, order_type, tif, match_type, flags));
  }

  void ModifyOrder(ClientID clientID, OrderID orderID, Price new_price,
                   Quantity new_quantity) {
    m_CommandQueue.push(
        CommandPayload::MakeModify(clientID, orderID, new_price, new_quantity));
  }

  void CancelOrder(ClientID clientID, OrderID orderID) {
    m_CommandQueue.push(CommandPayload::MakeCancel(clientID, orderID));
  }

  OrderBookSnapshot GetSnapshot() {
    auto *active = m_ActiveSnapshot.load(std::memory_order_acquire);
    return *active;
  }

  void UpdateSnapshot(u32 depth) {
    auto *curr = m_ActiveSnapshot.load(std::memory_order_relaxed);
    i32 inactive_idx = (curr == m_Snapshots[0].get()) ? 1 : 0;

    *m_Snapshots[inactive_idx] = m_Orderbook.GetSnapshot(depth);
    m_ActiveSnapshot.store(m_Snapshots[inactive_idx].get(),
                           std::memory_order_release);
  }

  [[nodiscard]]
  bool TryGetEvent(EventPayload &out) noexcept {
    return m_EventQueue.try_pop(out);
  }

  void HandleEvent(const EventPayload &event) {
    // TODO:
    using enum EventType;
    switch (event.GetType()) {
    case OrderAccepted:
    case OrderRejected:
    case ModifyAccepted:
    case ModifyRejected:
    case CancelAccepted:
    case CancelRejected:
      break;
    default:
      break;
    }
  }

private:
  Book m_Orderbook{};

  // TODO: replace with MPSC
  data::ThreadSafeQueue<CommandPayload> m_CommandQueue;
  data::SPSC_Queue<EventPayload, EventCapacity> m_EventQueue;

  std::shared_ptr<OrderBookSnapshot> m_Snapshots[2];
  std::atomic<OrderBookSnapshot *> m_ActiveSnapshot{nullptr};

  std::jthread m_Thread;

  void Run(std::stop_token st) {
    static u64 orders_processed = 0;
    static constexpr auto TickRate = std::chrono::nanoseconds{TickRateNs};

    while (!st.stop_requested()) [[likely]] {
      bool processed_any = false;

      CommandPayload cmd;
      while (m_CommandQueue.try_pop(cmd)) [[likely]] {
        orders_processed++;
        processed_any = true;
        auto events = m_Orderbook.Apply(cmd);

        for (const auto &event : events)
          HandleEvent(event);
      }

      // TODO: this is bad
      if (!processed_any)
        std::this_thread::sleep_for(TickRate);
    }

    CommandPayload cmd;
    while (m_CommandQueue.try_pop(cmd))
      auto events = m_Orderbook.Apply(cmd);

    std::cout << "Orders processed: " << orders_processed << std::endl;
  }
};

// Trading Engine Factory
template <class MatchingStrategy, u64 TickRateNs = 100'000,
          u64 CommandCapacity = (2 << 16), u64 EventCapacity = (2 << 16)>
[[nodiscard]] auto MakeTradingEngine() {
  using Engine = TradingEngine<MatchingStrategy, TickRateNs, CommandCapacity,
                               EventCapacity>;
  return std::make_unique<Engine>();
}
} // namespace ob::engine
