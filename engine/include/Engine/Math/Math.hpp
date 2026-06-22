#pragma once

#include <cmath>

namespace e00 {

#if defined(__DJGPP__)
/**
 * Portable log2 wrapper for DJGPP which might lack it in std::
 */
inline double log2(double x) {
    return ::log(x) / 0.69314718055994530941723212145818;
}

/**
 * Portable lrint wrapper for DJGPP which might lack it in std::
 */
inline long lrint(double x) {
    return static_cast<long>(::floor(x + 0.5));
}
#else
/**
 * Use standard implementation on other platforms
 */
using std::log2;
using std::lrint;
#endif

} // namespace e00
