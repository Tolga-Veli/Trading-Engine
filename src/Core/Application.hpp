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
  std::chrono::milliseconds m_FrameTime{20};

  engine::TradingEngine<engine::FIFO_Matching> m_TradingEngine;
  render::Renderer m_Renderer;

  void HandleEvent(const engine::Event &event);
};

}; // namespace ob
