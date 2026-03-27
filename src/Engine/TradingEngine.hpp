#pragma once

#include "CommandQueue.hpp"
#include "EventQueue.hpp"
#include "OrderBook.hpp"

namespace ob::engine {
template <class MatchingStrategy> class TradingEngine {
public:
  using Book = OrderBook<MatchingStrategy>;

  TradingEngine()
      : m_PoolResource(), m_EventQueue(),
        m_Orderbook(m_EventQueue, m_PoolResource),
        m_CommandQueue(m_Orderbook) {};

  TradingEngine(const TradingEngine &) = delete;
  TradingEngine &operator=(const TradingEngine &) = delete;
  TradingEngine(TradingEngine &&) = delete;
  TradingEngine &operator=(TradingEngine &&) = delete;

  void Start() { m_CommandQueue.Start(); }

  Event GetEvent() {
    Event event;
    while (m_EventQueue.try_pop(event))
      return event;
  }

  void AddOrder(ClientID clientID, ClientOrderID clientOrderID, Price price,
                Quantity quantity, Side side, OrderType order_type,
                TimeInForce tif, Flags flags) {
    m_CommandQueue.PushCommand(CommandTypes::AddOrder{clientID, clientOrderID,
                                                      price, quantity, side,
                                                      order_type, tif, flags});
  }

  void ModifyOrder(OrderID orderID, Price new_price, Quantity new_quantity) {
    m_CommandQueue.PushCommand(
        CommandTypes::ModifyOrder{orderID, new_price, new_quantity});
  }

  void CancelOrder(OrderID orderID) {
    m_CommandQueue.PushCommand(CommandTypes::CancelOrder{orderID});
  }

  void RequestSnapshot(std::uint32_t depth) {
    m_CommandQueue.PushCommand(CommandTypes::RequestSnapshot{depth});
  }

private:
  std::pmr::unsynchronized_pool_resource m_PoolResource;

  EventQueue m_EventQueue;
  Book m_Orderbook;
  CommandQueue<Book> m_CommandQueue; // must be destructed after orderbook
};
} // namespace ob::engine
