#pragma once

#include "CommandQueue.hpp"
#include "EventQueue.hpp"
#include "OrderBook.hpp"

namespace ob::engine {
class TradingEngine {
public:
  TradingEngine();

  TradingEngine(const TradingEngine &) = delete;
  TradingEngine &operator=(const TradingEngine &) = delete;
  TradingEngine(TradingEngine &&) = delete;
  TradingEngine &operator=(TradingEngine &&) = delete;

  void Start() { m_CommandQueue.Start(); }

  EventQueue &GetEventQueue() noexcept { return m_EventQueue; }
  CommandQueue &GetCommandQueue() noexcept { return m_CommandQueue; }

private:
  std::pmr::unsynchronized_pool_resource m_PoolResource;

  EventQueue m_EventQueue;
  OrderBook m_Orderbook;
  CommandQueue m_CommandQueue;
};
} // namespace ob::engine
