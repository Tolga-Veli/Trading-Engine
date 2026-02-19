#include <random>
#include <thread>

#include "Application.hpp"

namespace ob {
template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

Application::Application() : m_TradingEngine() {};

Application::~Application() { m_Running = false; }

void Application::Run() {
  static std::mt19937 rng{123};
  static std::uniform_int_distribution<uint32_t> dist(1, 1000);
  static std::size_t timer1 = 1, timer2 = 1;

  m_TradingEngine.Start();
  m_TradingEngine.GetCommandQueue().RequestSnapshot(30);

  while (m_Running) {
    engine::Event event;
    while (m_TradingEngine.GetEventQueue().try_pop(event)) {
      std::visit(
          overloaded{
              [](std::monostate) {},
              [this](const engine::EventTypes::SnapshotRequestAccepted &e) {
                m_Renderer.Render(e.snapshot);
              },
              [](const auto &other_events) {
                // Handle OrderAccepted, Cancelled, etc., or just ignore for
                // rendering
              }},
          event);
    }

    m_TradingEngine.GetCommandQueue().AddOrder(
        ClientID{1}, ClientOrderID{timer1++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Buy, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);
    m_TradingEngine.GetCommandQueue().AddOrder(
        ClientID{2}, ClientOrderID{timer2++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Sell, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);

    m_TradingEngine.GetCommandQueue().RequestSnapshot(30);

    std::this_thread::sleep_for(m_FrameTime);
  }
}

} // namespace ob
