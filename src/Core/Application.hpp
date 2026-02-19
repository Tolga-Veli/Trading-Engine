#pragma once

#include "Renderer/Renderer.hpp"
#include "TradingEngine.hpp"

namespace ob {
class Application {
public:
  Application();
  ~Application();

  void Run();

private:
  bool m_Running = true;
  std::chrono::milliseconds m_FrameTime{20};

  engine::TradingEngine m_TradingEngine;
  render::Renderer m_Renderer;
};

}; // namespace ob
