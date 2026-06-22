#pragma once

#ifdef WIN32
#include <Windows.h>
#include <string>

inline std::wstring s2ws(const std::string &s) {
  auto str_length = (int)s.length() + 1;
  auto len = MultiByteToWideChar(
    CP_ACP,
    0,
    s.c_str(),
    str_length,
    nullptr,
    0);

  std::wstring r;
  r.resize(len);

  MultiByteToWideChar(
    CP_ACP,
    0,
    s.c_str(),
    str_length,
    r.data(),
    r.length());

  return r;
}
#endif