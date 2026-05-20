#pragma once

#include "OrderBookSnapshot.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace Hermes::render {
class Renderer {
public:
  Renderer();
  ~Renderer();

  void Render(const engine::OrderBookSnapshot &snapshot);

private:
  i32 m_LastHeight = 0;

  bool m_FirstRender = true;
  bool m_Running = false;
};
} // namespace Hermes::render
