#include "Application.hpp"
#include "Memory/MemoryUtility.hpp"

int main() {
  std::unique_ptr<ob::Application> app =
      std::make_unique<ob::Application>(64 * ob::Memory::MiB);
  app->Run();
}
