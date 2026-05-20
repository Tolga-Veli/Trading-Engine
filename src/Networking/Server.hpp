#pragma once

#include "Networking.hpp"

#include <atomic>
#include <netinet/in.h>
#include <thread>

namespace ob::networking {
class Server {
public:
  Server() = default;

  void Start() { m_Running.store(true, std::memory_order_acquire); }

  void Close() { m_Running.store(false, std::memory_order_acquire); }

  void Submit(const Request &request);

  bool TryGetResponse(Response &response);

private:
  std::atomic<bool> m_Running{false};
  std::jthread m_Thread;

  void Run() {
    while (m_Running.load(std::memory_order_acquire)) {
      Request req;
    }
  }
};
}; // namespace ob::networking
//
