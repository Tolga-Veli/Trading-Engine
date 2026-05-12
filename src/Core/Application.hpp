#pragma once

#include "Engine/Orderbook.hpp"
#include "Renderer/Renderer.hpp"

namespace ob {
class Application {
public:
  Application();
  ~Application();

  void Init();
  void Run();
  void Shutdown();

private:
  engine::Orderbook m_Orderbook;
  render::Renderer m_Renderer;
  bool m_Running = false;
  static constexpr std::chrono::milliseconds m_FrameTime{33};

  void Tick();
};

}; // namespace ob
