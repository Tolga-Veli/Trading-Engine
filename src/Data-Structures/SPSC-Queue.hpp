#pragma once

#include "Core/globals.hpp"

#include <array>
#include <atomic>
#include <concepts>

namespace Hermes::data {
template <class T, u32 Size>
  requires(Size > 0 && (Size & (Size - 1)) == 0)
class SPSC_Queue {
public:
  SPSC_Queue() = default;
  ~SPSC_Queue() = default;

  SPSC_Queue(const SPSC_Queue &) = delete;
  SPSC_Queue &operator=(const SPSC_Queue &) = delete;
  SPSC_Queue(SPSC_Queue &&) = delete;
  SPSC_Queue &operator=(SPSC_Queue &&) = delete;

  template <class U>
    requires std::constructible_from<T, U>
  bool push(U &&val) {
    const auto curr_tail = m_TailCache;
    const auto next_head = (m_Head + 1) & BitMask;

    if (next_head == curr_tail) {
      m_TailCache = m_Tail.load(std::memory_order_acquire);
      if (next_head == m_TailCache)
        return false; // Full
    }

    m_Buffer[m_Head] = std::forward<U>(val);
    m_Head.store(next_head, std::memory_order_release);
    return true;
  }

  bool pop(T &out) {
    const auto curr_head = m_HeadCache;

    if (m_Tail == curr_head) {
      m_HeadCache = m_Head.load(std::memory_order_acquire);
      if (m_Tail == m_HeadCache)
        return false;
    }

    out = std::move(m_Buffer[m_Tail]);

    // Release memory order guarantees the producer sees the slot is freed
    m_Tail.store((m_Tail + 1) & BitMask, std::memory_order::release);
    return true;
  }

private:
  static constexpr u32 BitMask = Size - 1;

  std::array<T, Size> m_Buffer{};

  alignas(64) std::atomic<u32> m_Head{0};
  u32 m_TailCache{0};

  alignas(64) std::atomic<u32> m_Tail{0};
  u32 m_HeadCache{0};
};
} // namespace Hermes::data
