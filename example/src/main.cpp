
#include "ExampleGame.hpp"
#include <Engine.hpp>

#ifdef WIN32
INT WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    PWSTR pCmdLine,
    int nCmdShow) {
#else
int main(int, char **) {
#endif

  if (const auto ec = e00::Init()) {
    // Error occurred, oh well, this is an example
    return EXIT_FAILURE;
  }

  auto engine = std::make_unique<ExampleGame>();
  e00::Run(*engine);
  e00::Exit();

  return EXIT_SUCCESS;
}
