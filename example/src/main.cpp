#include "ExampleGame.hpp"
#include <Engine.hpp>

namespace {
int RunExampleGame() {
  if (const auto ec = e00::Init()) {
    // Error occurred, oh well, this is an example
    return 1;
  }

  auto engine = std::make_unique<ExampleGame>();
  e00::Run(*engine);
  e00::Exit();
  return 0;
}
} // namespace

#if defined(WIN31)
// Freestanding Win32s entry (Backend_Win31/EntryPoint.cpp) calls this.
extern "C" int e00_app_main() {
  return RunExampleGame();
}
#elif defined(WIN32)
INT WINAPI wWinMain(
    HINSTANCE /*hInstance*/,
    HINSTANCE /*hPrevInstance*/,
    PWSTR /*pCmdLine*/,
    int /*nCmdShow*/) {
  return RunExampleGame();
}
#else
int main(int, char **) {
  return RunExampleGame();
}
#endif
