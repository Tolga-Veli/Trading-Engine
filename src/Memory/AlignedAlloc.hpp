#pragma once

#include <cstddef>
#include <cstdlib>

#if defined(_WIN32)
#include <malloc.h>
#endif

#include "MemoryUtility.hpp"

namespace ob::Memory {

inline void *aligned_malloc(std::size_t size, std::size_t alignment) {
  if (size == 0)
    return nullptr;

#if !defined(_WIN32)
  if (alignment < alignof(void *))
    alignment = alignof(void *);
#endif

  if (!IsPow2(alignment))
    return nullptr;

#if defined(_WIN32)
  return _aligned_malloc(size, alignment);
#else
  void *ptr = nullptr;
  if (posix_memalign(&ptr, alignment, size))
    return nullptr;
  return ptr;
#endif
}

inline void aligned_free(void *ptr) noexcept {
  if (!ptr)
    return;

#if defined(_WIN32)
  _aligned_free(ptr);
#else
  std::free(ptr);
#endif
}
} // namespace ob::Memory
