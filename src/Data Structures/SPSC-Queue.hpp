#pragma once

#include "Core/globals.hpp"

#include <atomic>

namespace ob::data {
template <class T, u32 size> class SPSC_Queue {
public:
  SPSC_Queue() = default;
  ~SPSC_Queue() = default;

  SPSC_Queue(const SPSC_Queue &) = default;
  SPSC_Queue &operator=(const SPSC_Queue &) = default;
  SPSC_Queue(SPSC_Queue &&) = default;
  SPSC_Queue &operator=(SPSC_Queue &&) = default;

  template <class U>
    requires std::is_constructible_v<U, T>
  bool push(U val) {
    const auto next = (m_Head + 1) % size;
    if (next == m_Tail.load())
      return false; // full

    m_Buffer[m_Head] = val;
    m_Head = next;
    return true;
  }

  bool pop(T &out) {
    if (m_Tail.load(std::memory_order::relaxed) == m_Head)
      return false;

    out = m_Buffer[m_Tail];
    m_Tail.store((m_Tail + 1) % size, std::memory_order::release);
    return true;
  }

private:
  std::array<T, size> m_Buffer{};
  u32 m_Head{0};
  std::atomic<u32> m_Tail{0};
};
} // namespace ob::data
