#pragma once

#include "Core/Logging.hpp"
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
    HERMES_INFO("Wrote backup files to: {}", m_Backup.string());
  }

  void WriteEventLog(const std::vector<std::byte> &buffer) const {
    core::WriteToFile(buffer, m_EventLog);
    HERMES_INFO("Wrote event logs to: {}", m_EventLog.string());
  }

private:
  fs::path m_Backup, m_EventLog;
};
} // namespace ob::engine
