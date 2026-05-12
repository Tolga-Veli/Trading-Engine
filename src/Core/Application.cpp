#include <random>
#include <thread>

#include "Application.hpp"

namespace ob {
Application::Application() { Init(); };
Application::~Application() { Shutdown(); }

void Application::Init() {
  m_Running = true;
  m_Renderer.Init();
}

void Application::Run() {
  static std::mt19937 rng{1};
  static std::uniform_int_distribution<uint32_t> dist(1, 1000);

  std::vector<OrderID> ids;
  while (m_Running) {
    Tick();
    auto id1 = m_Orderbook.AddOrder(
        ClientID{1}, Price{dist(rng)}, Quantity{dist(rng)}, ob::Side::Buy,
        ob::OrderType::Limit, ob::TimeInForce::GoodTillCancelled,
        ob::Flags::None);
    auto id2 = m_Orderbook.AddOrder(
        ClientID{2}, Price{dist(rng)}, Quantity{dist(rng)}, ob::Side::Sell,
        ob::OrderType::Limit, ob::TimeInForce::GoodTillCancelled,
        ob::Flags::None);
    ids.push_back(id1);
    ids.push_back(id2);

    if (ids.size() % 50 == 0) {
      std::vector<OrderID> cancel;
      for (auto order_id : ids) {
        if (cancel.size() >= 10)
          break;

        auto order = m_Orderbook.GetOrder(order_id);
        if (order.has_value())
          cancel.push_back(order_id);
      }

      for (auto order_id : cancel)
        m_Orderbook.CancelOrder(order_id);
    }
    std::this_thread::sleep_for(m_FrameTime);
    // m_Renderer.Clear();
  }
}

void Application::Shutdown() {
  m_Running = false;
  m_Renderer.Shutdown();
}

void Application::Tick() {
  auto snapshot = m_Orderbook.GetSnapshot(10);
  m_Renderer.Render(snapshot);
}
} // namespace ob
