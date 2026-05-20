#pragma once

#include "SocketCommon.h"

#include <memory>
#include <vector>

namespace Hermes::network {
class TcpConnection {
public:
  explicit TcpConnection(socket_t clientSocket) : m_Socket(clientSocket) {}
  ~TcpConnection() {
    if (IsValid())
      CloseSocketHandle(m_Socket);
  }

  TcpConnection(const TcpConnection &) = delete;
  TcpConnection &operator=(const TcpConnection &) = delete;
  TcpConnection(TcpConnection &&) = delete;
  TcpConnection &operator=(TcpConnection &&) = delete;

  i32 Receive(char *buffer, u64 bufferSize) {
    i32 bytes = ::recv(m_Socket, buffer, static_cast<i32>(bufferSize), 0);
    if (bytes < 0)
      throw SocketException("Read operation", GetLastError());
    return bytes;
  }

  bool IsValid() const { return m_Socket != InvalidSocket; }

private:
  socket_t m_Socket;
};

class TcpServer {
public:
  TcpServer() : m_ListeningSocket(InvalidSocket) {}
  ~TcpServer() { Stop(); }

  void Start(i32 port) {
    m_ListeningSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_ListeningSocket == InvalidSocket)
      throw SocketException("Socket creation", GetLastError());

    i32 opt = 1;
    ::setsockopt(m_ListeningSocket, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char *>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(m_ListeningSocket, reinterpret_cast<struct sockaddr *>(&addr),
               sizeof(addr)) == SocketError) {
      i32 err = GetLastError();
      Stop();
      throw SocketException("Binding port " + std::to_string(port), err);
    }

    if (::listen(m_ListeningSocket, SOMAXCONN) == SocketError) {
      i32 err = GetLastError();
      Stop();
      throw SocketException("Listening setup", err);
    }
  }

  std::unique_ptr<TcpConnection> AcceptNextClient() {
    if (m_ListeningSocket == InvalidSocket)
      throw std::runtime_error(
          "Server must be running before calling AcceptNextClient");

    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);
    socket_t clientSocket =
        ::accept(m_ListeningSocket,
                 reinterpret_cast<struct sockaddr *>(&clientAddr), &addrLen);

    if (clientSocket == InvalidSocket)
      throw SocketException("Accept client link", GetLastError());

    return std::make_unique<TcpConnection>(clientSocket);
  }

  void Stop() {
    if (m_ListeningSocket != InvalidSocket) {
      CloseSocketHandle(m_ListeningSocket);
      m_ListeningSocket = InvalidSocket;
    }
  }

private:
  socket_t m_ListeningSocket;
};
}; // namespace Hermes::network
//
