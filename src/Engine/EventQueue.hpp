#pragma once

#include "Events.hpp"
#include "ThreadSafeQueue.hpp"

namespace ob::engine {

class EventQueue {
public:
  explicit EventQueue() = default;

  ~EventQueue() { m_Queue.close(); }

  EventQueue(const EventQueue &) = delete;
  EventQueue &operator=(const EventQueue &) = delete;
  EventQueue(EventQueue &&) = delete;
  EventQueue &operator=(EventQueue &&) = delete;

  template <class U> void push(U &&event) {
    m_Queue.push(std::forward<U>(event));
  }

  bool try_pop(Event &other) { return m_Queue.try_pop(other); }

private:
  data::ThreadSafeQueue<Event> m_Queue;
};
} // namespace ob::engine
