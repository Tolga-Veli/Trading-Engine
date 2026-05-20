#pragma once

#include "Core/globals.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

namespace Hermes::network {
using socket_t = SOCKET;
constexpr socket_t InvalidSocket = INVALID_SOCKET;
constexpr i32 SocketError = SOCKET_ERROR;

inline i32 GetLastError() { return ::WSAGetLastError(); }
inline void CloseSocketHandle(socket_t s) { ::closesocket(s); }
} // namespace Hermes::network

#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace Hermes::network {
using socket_t = i32;
constexpr socket_t InvalidSocket = -1;
constexpr i32 SocketError = -1;

inline i32 GetLastError() { return errno; }
inline void CloseSocketHandle(socket_t s) { ::close(s); }
} // namespace Hermes::network
#endif

#include <stdexcept>
#include <string>

namespace Hermes::network {
class NetworkPlatform {
public:
  static bool Init() {
#ifdef _WIN32
    WSADATA wsaData;
    return ::WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true;
#endif
  }

  static void Shutdown() {
#ifdef _WIN32
    ::WSACleanup();
#endif
  }
};

class SocketException : public std::runtime_error {
public:
  SocketException(const std::string &action, i32 errorCode)
      : std::runtime_error(
            action + "failed with OS error code: " + std::to_string(errorCode)),
        m_ErrorCode(errorCode) {}

  i32 GetErrorCode() const noexcept { return m_ErrorCode; }

private:
  i32 m_ErrorCode;
};
} // namespace Hermes::network
