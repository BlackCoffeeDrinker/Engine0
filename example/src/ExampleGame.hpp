
#ifndef ENGINE00_EXAMPLEGAME_HPP
#define ENGINE00_EXAMPLEGAME_HPP

#include <Engine.hpp>

class ExampleGame : public e00::Engine {
public:
  std::string_view Name() const noexcept override;
};


#endif//ENGINE00_EXAMPLEGAME_HPP
