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

  engine::EventQueue &GetEventQueue() noexcept { return m_EventQueue; }
  engine::CommandQueue &GetCommandQueue() noexcept { return m_CommandQueue; }

private:
  engine::EventQueue m_EventQueue;
  engine::OrderBook m_Orderbook;
  engine::CommandQueue m_CommandQueue;
};
} // namespace ob::engine
