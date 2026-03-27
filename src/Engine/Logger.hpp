#pragma once

#include "Core/globals.hpp"

#include <filesystem>
#include <vector>

namespace ob::engine {
namespace fs = std::filesystem;

class Logger {
public:
  Logger() = delete;
  Logger(const fs::path backup, const fs::path event_log)
      : m_Backup(backup), m_EventLog(event_log) {}

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;
  Logger(Logger &&) = delete;
  Logger &operator=(Logger &&) = delete;

  void WriteBackup(const std::vector<std::byte> &buffer) const {
    core::WriteToFile(buffer, m_Backup);
  }

  void WriteEventLog(const std::vector<std::byte> &buffer) const {
    core::WriteToFile(buffer, m_EventLog);
  }

private:
  fs::path m_Backup, m_EventLog;
};
} // namespace ob::engine
