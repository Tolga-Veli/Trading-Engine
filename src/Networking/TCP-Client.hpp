#pragma once

#include "SocketCommon.h"

namespace Hermes::network {

class TcpClient {
public:
  TcpClient() : m_Socket(InvalidSocket) {}
  ~TcpClient() { Disconnect(); }

  void Connect(const std::string &ipAddress, int port) {
    m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (m_Socket == InvalidSocket)
      throw SocketException("Client socket initialization", GetLastError());

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = ::htons(port);

    if (::inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr) <= 0) {
      int err = GetLastError();
      Disconnect();
      throw SocketException("IP string validation for " + ipAddress, err);
    }

    if (::connect(m_Socket, reinterpret_cast<struct sockaddr *>(&serverAddr),
                  sizeof(serverAddr)) == SocketError) {
      int err = GetLastError();
      Disconnect();
      throw SocketException(
          "Connecting to " + ipAddress + ":" + std::to_string(port), err);
    }
  }

  int Send(const char *data, size_t size) {
    if (m_Socket == InvalidSocket)
      throw std::runtime_error("Cannot send data; client is not connected.");

    int bytesSent = ::send(m_Socket, data, static_cast<int>(size), 0);
    if (bytesSent == SocketError) {
      throw SocketException("Send operation", GetLastError());
    }
    return bytesSent;
  }

  void Disconnect() {
    if (m_Socket != InvalidSocket) {
      CloseSocketHandle(m_Socket);
      m_Socket = InvalidSocket;
    }
  }

private:
  socket_t m_Socket;
};
} // namespace Hermes::network
