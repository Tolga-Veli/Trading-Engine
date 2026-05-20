#pragma once

#include "globals.hpp"

#include <cstdint>
#include <format>
#include <iostream>
#include <string_view>

namespace Hermes::core {

enum class LogLevel : uint16_t { Trace = 0, Info, Warning, Error, Fatal, None };

class Logger {
public:
  static inline constexpr std::string_view HERMES_COL_GRAY = "\033[90m";
  static inline constexpr std::string_view HERMES_COL_GREEN = "\033[32m";
  static inline constexpr std::string_view HERMES_COL_YELLOW = "\033[33m";
  static inline constexpr std::string_view HERMES_COL_RED = "\033[31m";
  static inline constexpr std::string_view HERMES_COL_FATAL =
      "\033[41m\033[37m";

  static void SetLevel(LogLevel level) { s_currentLevel = level; }
  static LogLevel GetLevel() { return s_currentLevel; }

  template <typename... Args>
  static void Log(LogLevel level, std::string_view prefix,
                  std::string_view color, std::format_string<Args...> fmt,
                  Args &&...args) {
    if (level < s_currentLevel)
      return;

    std::string message = std::format(fmt, std::forward<Args>(args)...);
    std::cout << color << "[" << prefix << "] " << message << "\033[0m"
              << std::endl;
  }

  template <typename... Args>
  static void LogFileAndLine(LogLevel level, std::string_view prefix,
                             std::string_view color, std::string_view file,
                             i32 line, std::format_string<Args...> fmt,
                             Args &&...args) {
    if (level < s_currentLevel)
      return;

    std::string message = std::format(fmt, std::forward<Args>(args)...);
    std::cout << color << "[" << prefix << "] " << file << ":" << line << ": "
              << message << "\033[0m" << std::endl;
  }

private:
  static inline LogLevel s_currentLevel = LogLevel::Trace;
};

static inline consteval std::string_view strip_path(std::string_view path) {
  size_t last_slash = path.find_last_of("\\/");
  return (last_slash == std::string_view::npos) ? path
                                                : path.substr(last_slash + 1);
}

} // namespace Hermes::core

#define HERMES_DEBUG 1

#ifdef HERMES_DEBUG
#define HERMES_INFO(...)                                                       \
  ::Hermes::core::Logger::Log(::Hermes::core::LogLevel::Info, "INFO",          \
                              ::Hermes::core::Logger::HERMES_COL_GREEN,        \
                              __VA_ARGS__)
#define HERMES_WARN(...)                                                       \
  ::Hermes::core::Logger::Log(::Hermes::core::LogLevel::Warning, "WARN",       \
                              ::Hermes::core::Logger::HERMES_COL_YELLOW,       \
                              __VA_ARGS__)
#else
#define HERMES_INFO(...)
#define HERMES_WARN(...)
#endif

#define HERMES_ERROR(...)                                                      \
  ::Hermes::core::Logger::LogFileAndLine(                                      \
      ::Hermes::core::LogLevel::Error, "ERROR",                                \
      ::Hermes::core::Logger::HERMES_COL_RED,                                  \
      ::Hermes::core::strip_path(__FILE__), __LINE__, __VA_ARGS__)

#define HERMES_FATAL(...)                                                      \
  do {                                                                         \
    ::Hermes::core::Logger::LogFileAndLine(                                    \
        ::Hermes::core::LogLevel::Fatal, "FATAL",                              \
        ::Hermes::core::Logger::HERMES_COL_FATAL,                              \
        ::Hermes::core::strip_path(__FILE__), __LINE__, __VA_ARGS__);          \
    std::abort();                                                              \
  } while (0)
