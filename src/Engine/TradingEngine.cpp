#include "TradingEngine.hpp"

namespace ob::engine {
TradingEngine::TradingEngine()
    : m_PoolResource(), m_EventQueue(),
      m_Orderbook(m_EventQueue, m_PoolResource), m_CommandQueue(m_Orderbook) {};

} // namespace ob::engine
