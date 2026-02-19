#pragma once

#include <cstddef>

namespace ob::Memory {

inline bool IsPow2(std::size_t size) {
  return size && ((size & (size - 1)) == 0);
}

inline std::size_t AlignUp(std::size_t size, std::size_t alignment) {
  return (size + (alignment - 1)) & ~(alignment - 1);
}

constexpr std::size_t DEFAULT_ALIGNMENT = 64;

inline constexpr std::size_t KiB = 1 << 10, MiB = 1 << 20, GiB = 1 << 30;
} // namespace ob::Memory
