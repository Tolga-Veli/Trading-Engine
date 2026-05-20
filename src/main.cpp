#include "Core/globals.hpp"
#include "Engine/TradingEngine.hpp"
#include "Renderer/Renderer.hpp"

constexpr ob::u32 fast_rand(ob::u32 i, ob::u32 seed) {
  i ^= seed;
  i *= 747796405u;
  i ^= (i >> 16);
  i *= 2891336453u;
  i ^= (i >> 16);
  return i;
}

int main() {
  using namespace ob;

  constexpr std::chrono::microseconds EngineTickRate{100};
  constexpr std::chrono::milliseconds FrameTime{50};
  constexpr std::chrono::seconds RunningTime{3};

  std::chrono::time_point<std::chrono::steady_clock> LastRenderTime{
      std::chrono::steady_clock::now()};

  auto TradingEngine =
      engine::MakeTradingEngine<matching::PriceTimePriority,
                                static_cast<u64>(EngineTickRate.count())>();
  TradingEngine->Start();

  auto start = std::chrono::steady_clock::now();

  u32 cnt = 0;
  render::Renderer Renderer;

  while (true) {
    if (std::chrono::steady_clock::now() - start >= RunningTime) {
      std::cout << "Hit 3-second timeout!" << std::endl;
      break;
    }

    if (const auto now = std::chrono::steady_clock::now();
        now - LastRenderTime > FrameTime) {
      TradingEngine->UpdateSnapshot(100);
      Renderer.Render(TradingEngine->GetSnapshot());
      LastRenderTime = now;
    }

    TradingEngine->AddOrder(ClientID{1}, Price{fast_rand(cnt, 3) % 1000 + 1},
                            Quantity{fast_rand(cnt, 4) % 1000 + 1}, Side::Buy,
                            OrderType::Limit, TimeInForce::GTC,
                            MatchType::Standard, Flags::None);

    TradingEngine->AddOrder(
        ClientID{2}, Price{fast_rand(cnt + 1, 5) % 1000 + 1},
        Quantity{fast_rand(cnt + 1, 6) % 1000 + 1}, Side::Sell,
        OrderType::Limit, TimeInForce::GTC, MatchType::Standard, Flags::None);

    cnt += 2;
  }

  std::cout << "Sent orders: " << cnt << std::endl;

  TradingEngine->Stop();
}
