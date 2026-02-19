#pragma once

#include "CommandQueue.hpp"
#include "OrderBook.hpp"
#include "Renderer.hpp"

namespace ob {
class Application {
public:
  Application(std::uint64_t buffer_size);
  ~Application();

  void Run();

private:
  bool m_Running = false;
  std::chrono::milliseconds m_FrameTime{16};

  std::pmr::monotonic_buffer_resource m_Resource;
  engine::OrderBook m_Orderbook;
  engine::CommandQueue m_CommandQueue;
  render::Renderer m_Renderer;

  void Tick();
};

}; // namespace ob
