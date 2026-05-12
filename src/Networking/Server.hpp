#pragma once

#include "Networking.hpp"

namespace ob::networking {
class Server {
public:
  Server() = default;

  void Close();
  Response handle(const Request &request);

private:
};
}; // namespace ob::networking
//
