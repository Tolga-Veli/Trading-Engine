#pragma once

#include "Core/Order.hpp"
#include <list>
#include <map>

namespace ob::engine {
struct OrderPointer {
  std::pmr::list<Order>::iterator list_iterator;
  std::pmr::map<Price, std::pmr::list<Order>>::iterator map_iterator;
};
} // namespace ob::engine
