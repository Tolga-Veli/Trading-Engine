#include "Core/globals.hpp"
#include "Engine/TradingEngine.hpp"
#include "Renderer/Renderer.hpp"

#include <random>

int main() {
  using namespace ob;

  constexpr std::chrono::microseconds EngineTickRate{100};
  constexpr std::chrono::milliseconds FrameTime{50};
  std::mt19937 rng{123};
  std::uniform_int_distribution<uint32_t> dist{1, 1000};

  std::chrono::time_point<std::chrono::steady_clock> LastRenderTime{
      std::chrono::steady_clock::now()};

  auto TradingEngine =
      engine::MakeTradingEngine<matching::PriceTimePriority,
                                static_cast<u64>(EngineTickRate.count())>();
  TradingEngine->Start();

  bool m_Running{true};
  render::Renderer Renderer;
  while (m_Running) {
    if (const auto now = std::chrono::steady_clock::now();
        now - LastRenderTime > FrameTime) {
      TradingEngine->UpdateSnapshot(50);
      Renderer.Render(TradingEngine->GetSnapshot());
      LastRenderTime = now;
    }

    if (false) {
      TradingEngine->Stop();
      m_Running = false;
    }

    TradingEngine->AddOrder(ClientID{1}, Price{dist(rng)}, Quantity{dist(rng)},
                            Side::Buy, OrderType::Limit,
                            TimeInForce::GoodTillCancelled, Flags::None);

    TradingEngine->AddOrder(ClientID{2}, Price{dist(rng)}, Quantity{dist(rng)},
                            Side::Sell, OrderType::Limit,
                            TimeInForce::GoodTillCancelled, Flags::None);
  }
}
