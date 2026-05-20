#include "Networking/TCP-Client.hpp"
#include <iostream>

using namespace Hermes::network;

int main() {
  try {
    TcpClient client;
    int port = 9099;
    std::cout << "[Client] Connecting to server: " << port << "...\n";
    client.Connect("127.0.0.1", port);

    bool running = true;
    while (running) {
      std::string str;
      std::cin >> str;

      if (str == "SHUTDOWN") {
        client.Disconnect();
        break;
      }

      client.Send(str.c_str(), str.size());

      std::cout << "[Client] Mock Order sent successfully.\n";
    }
  } catch (const SocketException &ex) {
    std::cerr << "[Client Exception] " << ex.what() << "\n";
  }

  // 3. Global Cleanup
  NetworkPlatform::Shutdown();
}
