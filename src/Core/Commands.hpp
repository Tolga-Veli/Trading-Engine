#pragma once

#include "globals.hpp"

#include <variant>

namespace ob::engine {
enum class CommandType { None = 0, Add = 1, Modify, Cancel };

// Holds structs with command info
namespace CommandTypes {
struct AddOrder {
  ClientID clientID;
  Price price;
  Quantity quantity;
  Side side;
  OrderType order_type;
  TimeInForce tif;
  Flags flags;
};

struct ModifyOrder {
  ClientID clientID;
  OrderID orderID;
  Price new_price;
  Quantity new_quantity;
};

struct CancelOrder {
  ClientID clientID;
  OrderID orderID;
};

} // namespace CommandTypes

using Command =
    std::variant<std::monostate, CommandTypes::AddOrder,
                 CommandTypes::ModifyOrder, CommandTypes::CancelOrder>;

} // namespace ob::engine
