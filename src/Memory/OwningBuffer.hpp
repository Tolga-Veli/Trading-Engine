#pragma once

#include <cstddef>
#include <cstring>

#include "MemoryUtility.hpp"

namespace ob::Memory {
class OwningBuffer {
public:
  OwningBuffer() noexcept = default;
  explicit OwningBuffer(std::size_t size,
                        std::size_t alignment = DEFAULT_ALIGNMENT);
  ~OwningBuffer();

  OwningBuffer(const OwningBuffer &other) = delete;
  OwningBuffer &operator=(const OwningBuffer &other) = delete;

  OwningBuffer(OwningBuffer &&other) noexcept;
  OwningBuffer &operator=(OwningBuffer &&other) noexcept;

  void allocate(std::size_t size, std::size_t alignment = DEFAULT_ALIGNMENT);
  void release() noexcept;

  explicit operator bool() const noexcept { return m_Data != nullptr; }

  std::byte *data() noexcept { return m_Data; }
  const std::byte *data() const noexcept { return m_Data; }

  template <class T> T *as() noexcept {
    if (!m_Data)
      return nullptr;
    return reinterpret_cast<T *>(m_Data);
  }

  template <class T> const T *as() const noexcept {
    if (!m_Data)
      return nullptr;
    return reinterpret_cast<const T *>(m_Data);
  }

  void clear() noexcept { std::memset(m_Data, 0, m_Bytes); }

  std::size_t size() const noexcept { return m_Bytes; }
  std::size_t alignment() const noexcept { return m_Alignment; }

private:
  std::byte *m_Data = nullptr;
  std::size_t m_Bytes = 0, m_Alignment = DEFAULT_ALIGNMENT;
};
} // namespace ob::Memory
