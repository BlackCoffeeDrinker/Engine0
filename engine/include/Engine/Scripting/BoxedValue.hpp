#pragma once

#include "Engine/Detail/Property.hpp"

namespace e00::scripting {
using BoxedValue = Property;

inline BoxedValue void_var() {
  return BoxedValue(BoxedValue::VoidType());
}
}// namespace e00::scripting
