
#pragma once

#include <cmath>

namespace e00::detail {

inline double sqrt(double x) {
#if 0
    
  double result;
  __asm__ (
      "fsqrt"          // Computes square root of ST(0) and stores in ST(0)
      : "=t" (result)  // Output: Top of the FPU stack
      : "0" (x)        // Input: Load x onto FPU stack
  );
  return result;
#else
  return std::sqrt(x);
#endif
}

}// namespace e00::detail
