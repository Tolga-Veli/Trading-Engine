#pragma once

#include <atomic>
#include <thread>
#include <variant>

#include "Core/Commands.hpp"
#include "Core/Events.hpp"
#include "Core/FileStream.hpp"
#include "Core/OrderBookSnapshot.hpp"
#include "Data Structures/ThreadSafeQueue.hpp"

namespace ob::engine {

class AuditData {
public:
  using AuditDataVariant =
      std::variant<std::monostate, Command, AuditBookSnapshot, Event, Trade>;

  AuditData() = default;
  AuditData(const AuditDataVariant &v) : m_Variant(v) {}

  template <typename... Funcs> void Decompose(Funcs &&...f) const {
    std::visit(Overloaded{std::forward<Funcs>(f)...}, m_Variant);
  }

private:
  AuditDataVariant m_Variant;
};

class Audit {
public:
  Audit()
      : m_SnapshotsLog(m_LoggingFolder / "Snapshots.journal"),
        m_CommandsLog(m_LoggingFolder / "Commands.journal"),
        m_EventsLog(m_LoggingFolder / "Events.journal"),
        m_TradesLog(m_LoggingFolder / "Trades.journal") {}

  ~Audit() {
    m_Running.store(false, std::memory_order::acquire);
    if (m_AuditThread.joinable())
      m_AuditThread.join();
  }

  void Start() { m_Running.store(true, std::memory_order::acquire); }

  void Stop() {
    m_Running.store(false, std::memory_order::acquire);
    m_SnapshotsLog.WriteBufferToFile();
    m_CommandsLog.WriteBufferToFile();
    m_EventsLog.WriteBufferToFile();
    m_TradesLog.WriteBufferToFile();
  }

  void Run() {
    while (m_Running.load(std::memory_order::acq_rel)) {

      AuditData buf;
      while (!m_Queue.try_pop(buf)) {
        buf.Decompose(
            [](std::monostate) {},
            [this](const Command &cmd) { m_CommandsLog.WriteToBuffer(cmd); },
            [this](const AuditBookSnapshot &snapshot) {
              m_SnapshotsLog.WriteToBuffer(snapshot);
            },
            [this](const Event &event) { m_EventsLog.WriteToBuffer(event); },
            [this](const Trade &trade) { m_TradesLog.WriteToBuffer(trade); },
            [](const auto &other) {});
      }

      std::this_thread::sleep_for(m_TickRate);
    }
  }

private:
  const fs::path m_LoggingFolder{"Logging"};
  OutputFileStream m_SnapshotsLog, m_CommandsLog, m_EventsLog, m_TradesLog;

  data::ThreadSafeQueue<AuditData> m_Queue;

  std::atomic<bool> m_Running{false};
  std::chrono::milliseconds m_TickRate{1000};
  std::thread m_AuditThread;
};
} // namespace ob::engine
