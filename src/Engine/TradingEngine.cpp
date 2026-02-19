#include "TradingEngine.hpp"

namespace ob::engine {
TradingEngine::TradingEngine()
    : m_EventQueue(), m_Orderbook(m_EventQueue), m_CommandQueue(m_Orderbook) {};

} // namespace ob::engine
