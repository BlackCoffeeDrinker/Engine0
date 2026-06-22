#pragma once

#include <cstdint>
#include "Engine.hpp"

struct EGA {
  bool present;
  bool active;
  bool color_display;
  int video_ram;
};

struct VGA {
  bool present;
  bool active;
};

struct VideoInfo {
  EGA ega_info;
  VGA vga_info;
};

int video_get_current_mode();
void video_set_current_mode(int mode);

VideoInfo video_get_info();
EGA video_get_info_ega();
VGA video_get_info_vga();

e00::OutputViewport *video_get_screen();
