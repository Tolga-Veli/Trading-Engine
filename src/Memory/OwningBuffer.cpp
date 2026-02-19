#include "OwningBuffer.hpp"
#include "AlignedAlloc.hpp"

#include <stdexcept>

namespace ob::Memory {
OwningBuffer::OwningBuffer(std::size_t size, std::size_t alignment) {
  allocate(size, alignment);
}

OwningBuffer::~OwningBuffer() { release(); }

OwningBuffer::OwningBuffer(OwningBuffer &&other) noexcept
    : m_Data(other.m_Data), m_Bytes(other.m_Bytes),
      m_Alignment(other.m_Alignment) {
  other.m_Data = nullptr;
  other.m_Bytes = 0;
  other.m_Alignment = DEFAULT_ALIGNMENT;
}

OwningBuffer &OwningBuffer::operator=(OwningBuffer &&other) noexcept {
  if (this == &other)
    return *this;

  release();

  m_Data = other.m_Data;
  m_Bytes = other.m_Bytes;
  m_Alignment = other.m_Alignment;

  other.m_Data = nullptr;
  other.m_Bytes = 0;
  other.m_Alignment = DEFAULT_ALIGNMENT;

  return *this;
}

void OwningBuffer::allocate(std::size_t size, std::size_t alignment) {
  if (!IsPow2(alignment))
    throw std::logic_error("OwningBuffer alignment must be a power of two");

  release();

  if (size == 0) {
    m_Alignment = alignment;
    return;
  }

  m_Data = static_cast<std::byte *>(aligned_malloc(size, alignment));
  if (!m_Data)
    throw std::bad_alloc();

  m_Bytes = size;
  m_Alignment = alignment;
}

void OwningBuffer::release() noexcept {
  if (m_Data)
    aligned_free(m_Data);
  m_Data = nullptr;
  m_Bytes = 0;
  m_Alignment = DEFAULT_ALIGNMENT;
}
} // namespace ob::Memory
