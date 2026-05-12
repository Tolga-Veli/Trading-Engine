#pragma once

#include <memory>
#include <mutex>
#include <thread>

#include "Data Structures/SPSC-Queue.hpp"
#include "Data Structures/ThreadSafeQueue.hpp"
#include "OrderBook.hpp"

namespace ob::engine {
template <class MatchingStrategy, u64 TickRateNs, u64 CommandCapacity,
          u64 EventCapacity>
class alignas(64) TradingEngine {
public:
  using Book = OrderBook<MatchingStrategy>;

  explicit TradingEngine() noexcept = default;

  TradingEngine(const TradingEngine &) = delete;
  TradingEngine &operator=(const TradingEngine &) = delete;
  TradingEngine(TradingEngine &&) = delete;
  TradingEngine &operator=(TradingEngine &&) = delete;

  ~TradingEngine() = default;

  void Start() {
    m_Thread = std::jthread([this](std::stop_token st) { Run(st); });
  };

  void Stop() { m_Thread.request_stop(); }

  void AddOrder(ClientID clientID, Price price, Quantity quantity, Side side,
                OrderType order_type, TimeInForce tif, Flags flags) {
    m_CommandQueue.push(CommandTypes::AddOrder{clientID, price, quantity, side,
                                               order_type, tif, flags});
  }

  void ModifyOrder(ClientID clientID, OrderID orderID, Price new_price,
                   Quantity new_quantity) {
    m_CommandQueue.push(
        CommandTypes::ModifyOrder{clientID, orderID, new_price, new_quantity});
  }

  void CancelOrder(ClientID clientID, OrderID orderID) {
    m_CommandQueue.push(CommandTypes::CancelOrder{clientID, orderID});
  }

  // TODO: many clients read snapshot, one thread should update snapshot
  OrderBookSnapshot GetSnapshot() {
    std::lock_guard<std::mutex> lock(m_SnapshotMutex);
    return m_LatestSnapshot;
  }

  void UpdateSnapshot(u32 depth) {
    std::lock_guard<std::mutex> lock(m_SnapshotMutex);
    auto snapshot = m_Orderbook.GetSnapshot(depth);
    m_LatestSnapshot = std::move(snapshot);
  }

  [[nodiscard]]
  bool TryGetEvent(Event &out) noexcept {
    return m_EventQueue.try_pop(out);
  }

  void HandleEvent(const ob::engine::Event &event) {
    using namespace ob::engine::EventTypes;
    event.Decompose(
        [](std::monostate) {}, [](const OrderAccepted &event) {},
        [](const OrderExpired &event) {}, [](const OrderRejected &event) {},
        [](const ModifyAccepted &event) {}, [](const ModifyRejected &event) {},
        [](const CancelAccepted &event) {}, [](const CancelRejected &event) {},
        [](const auto &other_events) {});
  }

private:
  Book m_Orderbook{};
  data::ThreadSafeQueue<Command> m_CommandQueue; // TODO: replace with MPSC
  data::SPSC_Queue<Event, EventCapacity> m_EventQueue;

  OrderBookSnapshot m_LatestSnapshot{};
  std::mutex m_SnapshotMutex{};

  std::jthread m_Thread;

  void Run(std::stop_token st) {
    static constexpr auto TickRate = std::chrono::nanoseconds{TickRateNs};

    Command cmd;
    while (!st.stop_requested()) [[likely]] {
      const auto tick_start = std::chrono::steady_clock::now();

      while (m_CommandQueue.try_pop(cmd)) [[likely]] {
        auto events = m_Orderbook.Apply(cmd);
        for (const auto &event : events)
          HandleEvent(event);
      }

      std::this_thread::sleep_for(TickRate);
    }

    while (m_CommandQueue.try_pop(cmd))
      auto events = m_Orderbook.Apply(cmd);
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
