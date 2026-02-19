#pragma once

#include "globals.hpp"

#include <variant>

namespace ob::engine {
enum class CommandType { None = 0, Add = 1, Modify, Cancel };

// Holds structs with command info
namespace cmd {
struct AddOrder {
  ClientID clientID;
  ClientOrderID clientOrderID;
  Price price;
  Quantity quantity;
  Side side;
  OrderType order_type;
  TimeInForce tif;
  Flags flags;
};

struct ModifyOrder {
  OrderID orderID;
  Price new_price;
  Quantity new_quantity;
};

struct CancelOrder {
  OrderID orderID;
};
} // namespace cmd
//

using Command = std::variant<std::monostate, cmd::AddOrder, cmd::ModifyOrder,
                             cmd::CancelOrder>;

} // namespace ob::engine
