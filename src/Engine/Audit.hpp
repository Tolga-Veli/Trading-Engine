#pragma once

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

template <u64 TickRateMs = 1000> class Audit {
public:
  Audit()
      : m_SnapshotsLog(m_LoggingFolder / "Snapshots.journal"),
        m_CommandsLog(m_LoggingFolder / "Commands.journal"),
        m_EventsLog(m_LoggingFolder / "Events.journal"),
        m_TradesLog(m_LoggingFolder / "Trades.journal") {}

  ~Audit() { Stop(); }

  void Start() {
    if (m_Thread.joinable()) [[unlikely]]
      return;

    m_Thread = std::jthread([this](std::stop_token stoken) { Run(stoken); });
  }

  void Stop() {
    m_Queue.close();
    if (m_Thread.joinable()) {
      m_Thread.request_stop();
      m_Thread.join();
    }
  }

  void FlushAll() {
    m_SnapshotsLog.WriteBufferToFile();
    m_CommandsLog.WriteBufferToFile();
    m_EventsLog.WriteBufferToFile();
    m_TradesLog.WriteBufferToFile();
  }

  template <typename U> void Enqueue(U &&data) {
    m_Queue.push(AuditData{std::forward<U>(data)});
  }

  void Run(std::stop_token stoken) {
    static constexpr std::chrono::milliseconds TickRate =
        std::chrono::milliseconds{1000};

    AuditData data;
    while (!stoken.stop_requested() && m_Queue.wait_and_pop(data)) {
      data.Decompose(
          [](std::monostate) {},
          [this](const Command &cmd) { m_CommandsLog.WriteToBuffer(cmd); },
          [this](const AuditBookSnapshot &snapshot) {
            m_SnapshotsLog.WriteToBuffer(snapshot);
          },
          [this](const Event &event) { m_EventsLog.WriteToBuffer(event); },
          [this](const Trade &trade) { m_TradesLog.WriteToBuffer(trade); },
          [](const auto &other) {});

      const auto now = std::chrono::steady_clock::now();

      std::this_thread::sleep_until(now + TickRate);
    }
  }

private:
  const fs::path m_LoggingFolder{"Logging"};
  OutputFileStream m_SnapshotsLog, m_CommandsLog, m_EventsLog, m_TradesLog;

  data::ThreadSafeQueue<AuditData> m_Queue;
  std::jthread m_Thread;
};
} // namespace ob::engine
