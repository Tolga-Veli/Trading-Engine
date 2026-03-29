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

  m_TradingEngine.SetTickRate(std::chrono::microseconds{100});

  m_TradingEngine.OnTick([&]() {
    m_TradingEngine.AddOrder(
        ClientID{1}, ClientOrderID{timer1++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Buy, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);
    m_TradingEngine.AddOrder(
        ClientID{2}, ClientOrderID{timer2++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Sell, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);
  });

  m_TradingEngine.OnEvent(
      [&](const engine::Event &event) { HandleEvent(event); });

  m_TradingEngine.Start();

  while (m_Running) {
    if (auto now = std::chrono::high_resolution_clock::now();
        now - m_LastRenderTime > m_FrameTime) {
      m_TradingEngine.UpdateSnapshot(50);
      m_Renderer.Render(m_TradingEngine.GetSnapshot());
      m_LastRenderTime = now;
    }
    std::this_thread::sleep_for(m_FrameTime);
  }
}

void Application::HandleEvent(const engine::Event &event) {
  using namespace engine::EventTypes;
  std::visit(
      overloaded{
          [](std::monostate) {}, [](const OrderAccepted &event) {},
          [](const OrderExpired &event) {}, [](const OrderRejected &event) {},
          [](const ModifyAccepted &event) {},
          [](const ModifyRejected &event) {},
          [](const CancelAccepted &event) {},
          [](const CancelRejected &event) {}, [](const auto &other_events) {}},
      event);
}

} // namespace ob
