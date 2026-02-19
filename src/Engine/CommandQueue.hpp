#pragma once

#include "Commands.hpp"
#include "OrderBook.hpp"
#include "ThreadSafeQueue.hpp"

#include <thread>

namespace ob::engine {

class CommandQueue {
public:
  explicit CommandQueue(OrderBook &orderbook) : m_Orderbook(orderbook) {}

  ~CommandQueue() {
    m_Thread.request_stop();
    m_Queue.close();
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

  void AddOrder(ClientID clientID, ClientOrderID clientOrderID, Price price,
                Quantity quantity, Side side, OrderType order_type,
                TimeInForce tif, Flags flags) {
    if (m_Thread.get_stop_token().stop_requested())
      return;
    m_Queue.push(
        Command{CommandTypes::AddOrder{clientID, clientOrderID, price, quantity,
                                       side, order_type, tif, flags}});
  }

  void ModifyOrder(OrderID orderID, Price new_price, Quantity new_quantity) {
    if (m_Thread.get_stop_token().stop_requested())
      return;
    m_Queue.push(
        Command{CommandTypes::ModifyOrder{orderID, new_price, new_quantity}});
  }

  void CancelOrder(OrderID orderID) {
    if (m_Thread.get_stop_token().stop_requested())
      return;
    m_Queue.push(Command{CommandTypes::CancelOrder{orderID}});
  }

  void RequestSnapshot(std::uint32_t depth) {
    if (m_Thread.get_stop_token().stop_requested())
      return;
    m_Queue.push(Command{CommandTypes::RequestSnapshot{depth}});
  }

private:
  OrderBook &m_Orderbook;
  data::ThreadSafeQueue<Command> m_Queue;
  std::jthread m_Thread;

  void ProcessLoop(std::stop_token stoken) {
    Command cmd;
    while (!stoken.stop_requested() && m_Queue.wait_and_pop(cmd))
      m_Orderbook.Apply(cmd);
  }
};
} // namespace ob::engine
