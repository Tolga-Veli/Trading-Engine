#pragma once

#include "Renderer/Renderer.hpp"
#include "TradingEngine.hpp"

namespace ob {
class Application {
public:
  Application();
  ~Application();

  static std::unique_ptr<Application> Create() {
    return std::make_unique<Application>();
  }

  void Run();

private:
  bool m_Running = true;

  std::chrono::milliseconds m_EngineTickRate{10};
  std::chrono::milliseconds m_FrameTime{50};
  std::chrono::time_point<std::chrono::high_resolution_clock> m_LastRenderTime{
      std::chrono::high_resolution_clock::now()};

  engine::TradingEngine<matching::PriceTimePriority> m_TradingEngine;
  render::Renderer m_Renderer;

  void HandleEvent(const engine::Event &event);
};

}; // namespace ob
