#pragma once

#include "Commands.hpp"
#include "OrderBook.hpp"
#include "ThreadSafeQueue.hpp"
#include <thread>

namespace ob::engine {

class CommandQueue {
public:
  explicit CommandQueue(OrderBook &orderbook)
      : m_Orderbook(orderbook), m_Thread(&CommandQueue::ProcessLoop, this) {}

  ~CommandQueue() {
    m_Running.store(false, std::memory_order_relaxed);
    m_Queue.close();
    if (m_Thread.joinable())
      m_Thread.join();
  }

  CommandQueue(const CommandQueue &) = delete;
  CommandQueue &operator=(const CommandQueue &) = delete;
  CommandQueue(CommandQueue &&) = delete;
  CommandQueue &operator=(CommandQueue &&) = delete;

  void AddOrder(ClientID clientID, ClientOrderID clientOrderID, Price price,
                Quantity quantity, Side side, OrderType order_type,
                TimeInForce tif, Flags flags) {
    if (!m_Running.load(std::memory_order_acquire))
      return;
    m_Queue.push(Command{cmd::AddOrder{clientID, clientOrderID, price, quantity,
                                       side, order_type, tif, flags}});
  }

  void ModifyOrder(OrderID orderID, Price new_price, Quantity new_quantity) {
    if (!m_Running.load(std::memory_order_acquire))
      return;
    m_Queue.push(Command{cmd::ModifyOrder{orderID, new_price, new_quantity}});
  }

  void CancelOrder(OrderID orderID) {
    if (!m_Running.load(std::memory_order_acquire))
      return;
    m_Queue.push(Command{cmd::CancelOrder{orderID}});
  }

private:
  OrderBook &m_Orderbook;
  data::ThreadSafeQueue<Command> m_Queue;
  std::atomic<bool> m_Running{true};
  std::thread m_Thread;

  void ProcessLoop() {
    Command cmd;
    while (m_Queue.wait_and_pop(cmd))
      Dispatch(cmd);
  }

  void Dispatch(const Command &cmd) { m_Orderbook.Apply(cmd); }
};
} // namespace ob::engine
