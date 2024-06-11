#pragma once

#include <cstdlib>

#ifdef _WIN32
#  if E00_LIBRARY_EXPORT == 1
#    define E00_API __declspec(dllexport)
#  else
#    define E00_API __declspec(dllimport)
#  endif
#else
#  define E00_API
#endif

// Global Configs
namespace e00 {
static constexpr size_t WORLD_PARTITIONING_MAX_ITEMS_PER_NODE = 4;
using WorldCoordinateType = uint16_t;
}
