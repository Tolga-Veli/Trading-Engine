#pragma once

#include "Core/globals.hpp"

#include <array>
#include <atomic>
#include <concepts>
#include <new>

namespace Hermes::data {
template <class T, u32 Size>
  requires(Size > 0 && (Size & (Size - 1)) == 0)
class MPSC_Queue {};
} // namespace Hermes::data
