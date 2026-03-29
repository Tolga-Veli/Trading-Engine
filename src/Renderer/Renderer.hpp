#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "OrderBookSnapshot.hpp"

namespace ob::render {
class Renderer {
public:
  Renderer();
  ~Renderer();

  void Render(const ob::engine::OrderBookSnapshot &snapshot);

private:
  i32 m_LastHeight = 0;

  bool m_FirstRender = true;
  bool m_Running = false;
};
} // namespace ob::render
