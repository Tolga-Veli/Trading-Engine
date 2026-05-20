#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace Hermes::data {
template <class T> class ThreadSafeQueue {
public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() { close(); }

  ThreadSafeQueue(const ThreadSafeQueue &) = delete;
  ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;
  ThreadSafeQueue(ThreadSafeQueue &&) = delete;
  ThreadSafeQueue &operator=(ThreadSafeQueue &&) = delete;

  void close() noexcept {
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      m_Closed = true;
    }
    m_CV.notify_all();
  }

  bool is_closed() const noexcept {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Closed;
  }

  template <class U> bool push(U &&value) {
    static_assert(std::is_constructible_v<T, U>,
                  "T must be constructible from U");
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      if (m_Closed)
        return false;
      m_Queue.emplace(std::forward<U>(value));
    }
    m_CV.notify_one();
    return true;
  }

  bool wait_and_pop(T &out) {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_CV.wait(lock, [this]() { return m_Closed || !m_Queue.empty(); });

    if (m_Queue.empty())
      return false;

    out = std::move(m_Queue.front());
    m_Queue.pop();
    return true;
  }

  bool try_pop(T &out) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Queue.empty())
      return false;

    out = std::move(m_Queue.front());
    m_Queue.pop();
    return true;
  }

  bool empty() const noexcept {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Queue.empty();
  }

  std::size_t size() const noexcept {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Queue.size();
  }

private:
  std::queue<T> m_Queue;
  mutable std::mutex m_Mutex;
  std::condition_variable m_CV;
  bool m_Closed{false};
};
}; // namespace Hermes::data
