#include "Application.hpp"

int main() {
  std::unique_ptr<ob::Application> app = std::make_unique<ob::Application>();
  app->Run();
}
