#pragma once

#include "globals.hpp"

#include <cassert>
#include <cstring>
#include <type_traits>

namespace ob::engine {
enum class CommandType : u8 { None = 0, Add = 1, Modify, Cancel };

namespace Commands {
struct alignas(8) Add {
  // 8-Byte Fields (24 Bytes)
  ClientID clientID;
  Price price;
  Quantity quantity;

  // 2-Byte Fields (2 Bytes)
  Flags flags;

  // 1-Byte Fields (4 Bytes)
  Side side;
  OrderType order_type;
  TimeInForce tif;
  MatchType match_type;

  // Explicit padding (2 Bytes)
  u8 pad[2]{};
};

static_assert(sizeof(Add) == 32, "Commands::Add size unexpected");
static_assert(std::is_trivially_copyable_v<Add>);

struct alignas(8) Modify {
  // 8-Byte Fields (32 Bytes)
  ClientID clientID;
  OrderID orderID;
  Price new_price;
  Quantity new_quantity;
};

static_assert(sizeof(Modify) == 32, "Commands::Modify size unexpected");
static_assert(std::is_trivially_copyable_v<Modify>);

struct alignas(8) Cancel {
  // 8-Byte Fields (16 Bytes)
  ClientID clientID;
  OrderID orderID;
};

static_assert(sizeof(Cancel) == 16, "Commands::Modify size unexpected");
static_assert(std::is_trivially_copyable_v<Cancel>);

} // namespace Commands

class alignas(8) CommandPayload {
public:
  CommandPayload() : m_CommandType(CommandType::None), m_Data{} {}

  [[nodiscard]] static CommandPayload
  MakeAdd(ClientID clientID, Price price, Quantity quantity, Side side,
          OrderType order_type, TimeInForce tif, MatchType match_type,
          Flags flags) noexcept {
    CommandPayload cmd;
    cmd.m_CommandType = CommandType::Add;
    cmd.m_Data.add = {
        clientID, price, quantity, flags, side, order_type, tif, match_type,
    };
    return cmd;
  }

  [[nodiscard]] static CommandPayload
  MakeModify(ClientID clientID, OrderID orderID, Price new_price,
             Quantity new_quantity) noexcept {
    CommandPayload cmd;
    cmd.m_CommandType = CommandType::Modify;
    cmd.m_Data.modify = {clientID, orderID, new_price, new_quantity};
    return cmd;
  }

  [[nodiscard]] static CommandPayload MakeCancel(ClientID clientID,
                                                 OrderID orderID) noexcept {
    CommandPayload cmd;
    cmd.m_CommandType = CommandType::Cancel;
    cmd.m_Data.cancel = {clientID, orderID};
    return cmd;
  }

  [[nodiscard]] CommandType GetType() const noexcept { return m_CommandType; }

  [[nodiscard]]
  constexpr const Commands::Add &AsAdd() const noexcept {
    assert(m_CommandType == CommandType::Add);
    return m_Data.add;
  }

  [[nodiscard]]
  constexpr const Commands::Modify &AsModify() const noexcept {
    assert(m_CommandType == CommandType::Modify);
    return m_Data.modify;
  }

  [[nodiscard]]
  constexpr const Commands::Cancel &AsCancel() const noexcept {
    assert(m_CommandType == CommandType::Cancel);
    return m_Data.cancel;
  }

  static inline void Serialize(std::byte *buffer,
                               const CommandPayload &cmd) noexcept {
    std::memcpy(buffer, &cmd, sizeof(cmd));
  }

  static inline CommandPayload Deserialize(const std::byte *src) noexcept {
    CommandPayload cmd;
    std::memcpy(&cmd, src, sizeof(CommandPayload));
    return cmd;
  }

private:
  // Takes the size of its largest member: Add/Modify (32 bytes)
  union {
    Commands::Add add;
    Commands::Modify modify;
    Commands::Cancel cancel;
  } m_Data;

  // 1 bytes
  CommandType m_CommandType;

  u8 m_Padding[7]{};
};

static_assert(sizeof(CommandPayload) == 40, "CommandPayload size unexpected");
static_assert(std::is_trivially_copyable_v<CommandPayload>);
} // namespace ob::engine
