
#pragma once
#include <cstdint>

namespace DOS {

enum class OSType {
  Type_UNKNOWN,
  Type_PURE_DOS,
  Type_WIN3,
  Type_WIN95,
  Type_WIN98,
  Type_WINME,
  Type_WINNT,
  Type_OS2,
  Type_WARP,
  Type_DOSEMU,
  Type_OPENDOS,
  Type_OPENDOS_EMM386,
  Type_DOSBOX_X,
};

struct OSInfo {
  OSType type = OSType::Type_UNKNOWN;
  uint8_t version = 0;
  uint8_t revision = 0;
};

OSInfo DetectOS();
}
