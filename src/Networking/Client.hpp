#pragma once

#include "Networking.hpp"

namespace ob::networking {
class Client {
public:
  Client() = default;

  void Connect();
  void Close();
  bool IsConnected();

  void SendRequest(const Request &request);

  bool TryGetResponse(Response &response);

private:
};

} // namespace ob::networking
