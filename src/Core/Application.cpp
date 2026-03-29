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
  static std::mt19937 rng{1};
  static std::uniform_int_distribution<uint32_t> dist(1, 1000);
  static std::size_t timer1 = 1, timer2 = 1;

  m_TradingEngine.Start();
  m_Renderer.Render({});

  while (m_Running) {
    while (true) {
      auto event = m_TradingEngine.GetEvent();
      if (std::holds_alternative<std::monostate>(event))
        break;

      HandleEvent(m_TradingEngine.GetEvent());
    }

    m_TradingEngine.AddOrder(
        ClientID{1}, ClientOrderID{timer1++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Buy, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);
    m_TradingEngine.AddOrder(
        ClientID{2}, ClientOrderID{timer2++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Sell, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);

    if (auto now = std::chrono::high_resolution_clock::now();
        now - m_LastRenderTime > m_FrameTime) {
      m_TradingEngine.UpdateSnapshot(30);
      auto snapshot = m_TradingEngine.GetSnapshot();
      m_Renderer.Render(snapshot);
      m_LastRenderTime = now;
    }

    std::this_thread::sleep_for(m_EngineTickRate);
  }
}

void Application::HandleEvent(const engine::Event &event) {
  std::visit(overloaded{[](std::monostate) {},
                        [](const auto &other_events) {
                          // Handle OrderAccepted, Cancelled, etc., or just
                          // ignore for rendering
                        }},
             event);
}

} // namespace ob
