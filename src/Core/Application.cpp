#include <random>
#include <thread>

#include "Application.hpp"

namespace ob {
Application::Application(std::uint64_t buffer_size)
    : m_Resource(buffer_size), m_Orderbook(m_Resource),
      m_CommandQueue(m_Orderbook), m_Renderer() {
  m_Running = true;
};

Application::~Application() { m_Running = false; }

void Application::Run() {
  static std::mt19937 rng{1};
  static std::uniform_int_distribution<uint32_t> dist(1, 1000);

  std::vector<OrderID> ids;
  static std::size_t timer1 = 1, timer2 = 1;
  while (m_Running) {
    Tick();
    m_CommandQueue.AddOrder(
        ClientID{1}, ClientOrderID{timer1++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Buy, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);
    m_CommandQueue.AddOrder(
        ClientID{2}, ClientOrderID{timer2++}, Price{dist(rng)},
        Quantity{dist(rng)}, ob::Side::Sell, ob::OrderType::Limit,
        ob::TimeInForce::GoodTillCancelled, ob::Flags::None);

    std::optional<std::size_t> id1 = 0, id2 = 0;
    if (id1.has_value())
      ids.push_back(id1.value());
    if (id2.has_value())
      ids.push_back(id2.value());

    std::this_thread::sleep_for(m_FrameTime);
  }
}

void Application::Tick() {
  auto snapshot = m_Orderbook.GetQueryAPI().GetSnapshot(30);
  m_Renderer.Render(snapshot);
}
} // namespace ob
