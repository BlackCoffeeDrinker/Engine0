#pragma once

#include <cstdint>

namespace e00 {

/**
 * 8-way facing direction, usable by any `World::Element` (not just mobile actors),
 * since it will also be needed later for FOV/vision checks.
 */
enum class Direction : uint8_t {
  North,
  NorthEast,
  East,
  SouthEast,
  South,
  SouthWest,
  West,
  NorthWest,
};

/**
 * Collapses an 8-way `Direction` to the nearest of the 4 cardinal directions
 * (North/East/South/West), for actor types that only author 4-direction sprite sets.
 *
 * @param facing the 8-way facing direction
 * @return the nearest cardinal direction
 */
[[nodiscard]] inline Direction CollapseTo4Way(Direction facing) noexcept {
  switch (facing) {
    case Direction::North:
    case Direction::NorthEast:
    case Direction::NorthWest:
      return Direction::North;
    case Direction::East:
      return Direction::East;
    case Direction::South:
    case Direction::SouthEast:
    case Direction::SouthWest:
      return Direction::South;
    case Direction::West:
      return Direction::West;
  }

  return Direction::South;
}

}// namespace e00
