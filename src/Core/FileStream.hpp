#pragma once

#include "globals.hpp"

#include <concepts>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Hermes::core::io {

inline std::string ReadFromFile(const std::filesystem::path &path) {
  std::ifstream in(path, std::ios::in | std::ios::binary | std::ios::ate);

  if (!in)
    return "";

  auto size = in.tellg();
  if (size <= 0)
    return "";

  std::string result;
  result.resize(size);

  in.seekg(0, std::ios::beg);
  if (!in.read(result.data(), size))
    return "";

  return result;
}

inline bool WriteToFile(const std::vector<std::byte> &buffer,
                        const std::filesystem::path &path) {
  std::ofstream out(path, std::ios::binary);

  if (!out)
    return false;

  out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
  return out.good();
}

namespace fs = std::filesystem;

template <class T>
  requires std::is_trivially_copyable_v<T>
inline void WriteRaw(std::vector<std::byte> &buffer, const T &val) {
  const auto *ptr = reinterpret_cast<const std::byte *>(&val);
  buffer.reserve(buffer.size() + sizeof(T));
  buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

template <class T>
  requires std::is_trivially_copyable_v<T>
inline void WriteArray(std::vector<std::byte> &buffer, const T *ptr, u64 count,
                       bool writeSize) {
  if (ptr == nullptr && count > 0)
    return;

  if (writeSize)
    WriteRaw(buffer, count);

  std::size_t old_sz = buffer.size(), new_bytes = count * sizeof(T);
  buffer.reserve(old_sz + new_bytes);

  const auto *byte_ptr = reinterpret_cast<const std::byte *>(ptr);
  buffer.insert(buffer.end(), byte_ptr, byte_ptr + count * sizeof(T));
}

inline void WriteString(std::vector<std::byte> &buffer, std::string_view str_v,
                        bool writeSize) {
  WriteArray(buffer, reinterpret_cast<const std::byte *>(str_v.data()),
             str_v.size(), writeSize);
}

inline void WriteString(std::vector<std::byte> &buffer, const std::string &str,
                        bool writeSize) {
  WriteArray(buffer, reinterpret_cast<const std::byte *>(str.data()),
             str.size(), writeSize);
}

template <class T, class Buffer>
concept Serializable = requires(Buffer &buffer, const T &val) {
  { T::Serialize(buffer, val) } -> std::same_as<void>;
};

class OutputFileStream {
public:
  OutputFileStream() = delete;
  OutputFileStream(fs::path path) : m_FilePath(path) {
    m_Stream.open(m_FilePath, std::ios::binary);

    if (!m_Stream.is_open())
      throw std::runtime_error("Failed to open file: " + m_FilePath.string());
  }
  ~OutputFileStream() {
    if (m_Stream.is_open())
      m_Stream.flush();
  }

  OutputFileStream(const OutputFileStream &) = delete;
  OutputFileStream &operator=(const OutputFileStream &) = delete;
  OutputFileStream(OutputFileStream &&) noexcept = default;
  OutputFileStream &operator=(OutputFileStream &&) noexcept = default;

  template <class T>
  void WriteToBuffer(const T &data)
    requires std::is_trivially_copyable_v<T> ||
             Serializable<T, std::vector<std::byte>>
  {
    if constexpr (std::is_trivially_copyable_v<T>)
      WriteRaw(m_Buffer, data);
    else
      T::Serialize(m_Buffer, data);
  }

  bool WriteBufferToFile() {
    if (m_Buffer.empty()) {
      m_Stream.flush();
      return true;
    }

    m_Stream.write(reinterpret_cast<const char *>(m_Buffer.data()),
                   m_Buffer.size());

    if (!m_Stream.good())
      return false;

    m_Buffer.clear();
    m_Stream.flush();
    return true;
  }

private:
  std::vector<std::byte> m_Buffer;
  fs::path m_FilePath;
  std::ofstream m_Stream;
};
} // namespace Hermes::core::io
