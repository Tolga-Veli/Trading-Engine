#pragma once

#include "CommandQueue.hpp"
#include "EventQueue.hpp"
#include "OrderBook.hpp"

#include <mutex>

namespace ob::engine {
template <class MatchingStrategy> class TradingEngine {
public:
  using Book = OrderBook<MatchingStrategy>;

  TradingEngine()
      : m_PoolResource(), m_EventQueue(), m_Orderbook(m_EventQueue),
        m_CommandQueue(m_Orderbook) {};

  TradingEngine(const TradingEngine &) = delete;
  TradingEngine &operator=(const TradingEngine &) = delete;
  TradingEngine(TradingEngine &&) = delete;
  TradingEngine &operator=(TradingEngine &&) = delete;

  void Start() { m_CommandQueue.Start(); }

  Event GetEvent() {
    Event event;
    while (m_EventQueue.try_pop(event))
      return event;
    return std::monostate{};
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

  OrderBookSnapshot GetSnapshot() {
    std::lock_guard<std::mutex> lock(m_SnapshotMutex);
    return m_LatestSnapshot;
  }

  void UpdateSnapshot(u32 depth) {
    auto snapshot = m_Orderbook.GetSnapshot(depth);
    std::lock_guard<std::mutex> lock(m_SnapshotMutex);
    m_LatestSnapshot = std::move(snapshot);
  }

private:
  std::pmr::unsynchronized_pool_resource m_PoolResource;

  EventQueue m_EventQueue;
  Book m_Orderbook;
  CommandQueue<Book> m_CommandQueue; // must be destructed after orderbook

  std::mutex m_SnapshotMutex;
  OrderBookSnapshot m_LatestSnapshot;
};
} // namespace ob::engine
