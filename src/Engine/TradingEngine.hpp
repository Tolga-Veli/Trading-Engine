#pragma once

#include <functional>
#include <mutex>

#include "CommandQueue.hpp"
#include "EventQueue.hpp"
#include "OrderBook.hpp"

namespace ob::engine {
template <class MatchingStrategy> class TradingEngine {
public:
  using Book = OrderBook<MatchingStrategy>;
  using EventCallback = std::function<void(const Event &)>;
  using TickCallback = std::function<void()>;

  TradingEngine()
      : m_PoolResource(), m_EventQueue(), m_Orderbook(m_EventQueue),
        m_CommandQueue(m_Orderbook) {};

  TradingEngine(const TradingEngine &) = delete;
  TradingEngine &operator=(const TradingEngine &) = delete;
  TradingEngine(TradingEngine &&) = delete;
  TradingEngine &operator=(TradingEngine &&) = delete;

  ~TradingEngine() { Stop(); }

  void OnEvent(EventCallback callback) {
    m_EventCallback = std::move(callback);
  }

  void OnTick(TickCallback callback) { m_TickCallback = std::move(callback); }

  void SetTickRate(std::chrono::nanoseconds rate) { m_TickRate = rate; }

  void Start() {
    m_CommandQueue.Start();
    m_Running.store(true, std::memory_order::release);
    m_Thread = std::thread([this] { Run(); });
  };

  void Stop() {
    m_Running.store(false, std::memory_order::release);
    if (m_Thread.joinable())
      m_Thread.join();
  }

  Event GetEvent() {
    Event event;
    while (m_EventQueue.try_pop(event))
      return event;
    return Event{};
  }

  void AddOrder(ClientID clientID, ClientOrderID clientOrderID, Price price,
                Quantity quantity, Side side, OrderType order_type,
                TimeInForce tif, Flags flags) {
    m_CommandQueue.PushCommand(CommandTypes::AddOrder{
        clientID, price, quantity, side, order_type, tif, flags});
  }

  void ModifyOrder(ClientID clientID, OrderID orderID, Price new_price,
                   Quantity new_quantity) {
    m_CommandQueue.PushCommand(
        CommandTypes::ModifyOrder{clientID, orderID, new_price, new_quantity});
  }

  void CancelOrder(ClientID clientID, OrderID orderID) {
    m_CommandQueue.PushCommand(CommandTypes::CancelOrder{clientID, orderID});
  }

  // TODO: many thread read snapshot, one thread should update snapshot
  OrderBookSnapshot GetSnapshot() {
    std::lock_guard<std::mutex> lock(m_SnapshotMutex);
    return m_LatestSnapshot;
  }

  void UpdateSnapshot(u32 depth) {
    std::lock_guard<std::mutex> lock(m_SnapshotMutex);
    auto snapshot = m_Orderbook.GetSnapshot(depth);
    m_LatestSnapshot = std::move(snapshot);
  }

private:
  std::pmr::unsynchronized_pool_resource m_PoolResource;

  EventQueue m_EventQueue;
  Book m_Orderbook;
  CommandQueue<Book> m_CommandQueue; // must be destructed after orderbook

  OrderBookSnapshot m_LatestSnapshot;
  std::mutex m_SnapshotMutex;

  EventCallback m_EventCallback;
  TickCallback m_TickCallback;
  std::chrono::nanoseconds m_TickRate{std::chrono::microseconds{100}};
  std::atomic<bool> m_Running{false};
  std::thread m_Thread;

  void Run() {
    while (m_Running.load(std::memory_order::relaxed)) {
      auto tick_start = std::chrono::high_resolution_clock::now();

      if (m_TickCallback)
        m_TickCallback();

      Event event;
      while (m_EventQueue.try_pop(event)) {
        if (m_EventCallback)
          m_EventCallback(event);
      }

      std::this_thread::sleep_until(tick_start + m_TickRate);
    }
  }
};
} // namespace ob::engine
