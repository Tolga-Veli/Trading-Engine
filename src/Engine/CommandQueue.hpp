#pragma once

#include "Commands.hpp"
#include "ThreadSafeQueue.hpp"

#include <thread>

namespace ob::engine {

template <class BookT> class CommandQueue {
public:
  explicit CommandQueue(BookT &orderbook) : m_Orderbook(orderbook) {}

  ~CommandQueue() {
    m_Queue.close();
    m_Thread.request_stop();
  }

  CommandQueue(const CommandQueue &) = delete;
  CommandQueue &operator=(const CommandQueue &) = delete;
  CommandQueue(CommandQueue &&) = delete;
  CommandQueue &operator=(CommandQueue &&) = delete;

  void Start() {
    if (!m_Thread.joinable())
      m_Thread =
          std::jthread([this](std::stop_token stoken) { ProcessLoop(stoken); });
  }

  template <class U> void PushCommand(U &&cmd) {
    static_assert(std::is_constructible<Command, U>::value,
                  "U must be constructible from Command");
    m_Queue.push(std::forward<U>(cmd));
  }

private:
  BookT &m_Orderbook;
  data::ThreadSafeQueue<Command> m_Queue;
  std::jthread m_Thread;

  void ProcessLoop(std::stop_token stoken) {
    Command cmd;
    while (!stoken.stop_requested() && m_Queue.wait_and_pop(cmd))
      m_Orderbook.Apply(cmd);
  }
};
} // namespace ob::engine
