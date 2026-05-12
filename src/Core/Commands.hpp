#pragma once

#include "globals.hpp"

#include <variant>

namespace ob::engine {
enum class CommandType { None = 0, Add = 1, Modify, Cancel };

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

class Command {
public:
  using CmdVariant =
      std::variant<std::monostate, CommandTypes::AddOrder,
                   CommandTypes::ModifyOrder, CommandTypes::CancelOrder>;

  Command() = default;
  Command(const CmdVariant &v) : m_Variant(v) {}

  constexpr CommandType GetType() const {
    return std::visit(
        [](auto &&arg) -> CommandType {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::monostate>)
            return CommandType::None;
          else if constexpr (std::is_same_v<T, CommandTypes::AddOrder>)
            return CommandType::Add;
          else if constexpr (std::is_same_v<T, CommandTypes::ModifyOrder>)
            return CommandType::Modify;
          else if constexpr (std::is_same_v<T, CommandTypes::CancelOrder>)
            return CommandType::Cancel;
        },
        m_Variant);
  }

  template <typename... Funcs> auto Decompose(Funcs &&...f) const {
    return std::visit(Overloaded{std::forward<Funcs>(f)...}, m_Variant);
  }

  static void Serialize(std::vector<std::byte> &buffer, const Command &cmd) {}

private:
  CmdVariant m_Variant;
};

inline void Serialize(std::vector<std::byte> &buffer, const Command &cmd) {}

} // namespace ob::engine
