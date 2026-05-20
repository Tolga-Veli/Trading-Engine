#include "Networking/TCP-Client.hpp"
#include "Networking/TCP-Server.hpp"
#include <iostream>
#include <thread>

using namespace Hermes::network;

int main() {
  if (!NetworkPlatform::Init()) {
    std::cerr << "Global system network initialization failed.\n";
    return 1;
  }

  try {
    TcpServer server;
    server.Start(9099);
    std::cout
        << "[Server] Order book engine server listening on port 9099...\n";

    // Blocks until our client connects
    auto connection = server.AcceptNextClient();
    std::cout << "[Server] Client session accepted!\n";

    char incomingBuffer[512];
    while (true) {
      int bytesRead =
          connection->Receive(incomingBuffer, sizeof(incomingBuffer));

      if (bytesRead == 0) {
        std::cout << "[Server] Client disconnected gracefully.\n";
        break;
      }

      std::cout << "[Server] Received order payload: ";
      std::cout.write(incomingBuffer, bytesRead) << "\n";
    }
  } catch (const SocketException &ex) {
    std::cerr << "[Server Exception] " << ex.what() << "\n";
  }

  NetworkPlatform::Shutdown();
}
