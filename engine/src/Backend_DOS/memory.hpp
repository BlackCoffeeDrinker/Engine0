
#pragma once

#include <stdint.h>

#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>


// Lock a range of code so it won't be paged out during interrupts.
// Usage: DOS_LockCode(function_name, function_end_label)
// The function_end_label must be defined immediately after the function.
#define DOS_LockCode(start, end) \
  _go32_dpmi_lock_code((void *) (start), (char *) (end) - (char *) (start))

// Lock a range of data so it won't be paged out during interrupts.
#define DOS_LockData(var, size) \
  _go32_dpmi_lock_data((void *) &(var), (size))

// Lock a single variable.
#define DOS_LockVariable(var) \
  DOS_LockData(var, sizeof(var))

// Set up for C function definitions, even when using C++
#ifdef __cplusplus
extern "C" {
#endif

// This uses the "fat DS" trick to convert a physical address to a valid
//  C pointer usable from protected mode.
inline void *DOS_PhysicalToLinear(const uint32_t physical) {
  __djgpp_nearptr_enable();// We need to re-enable this for large applications to work.
  return reinterpret_cast<void *>(physical + __djgpp_conventional_base);
}

inline uint32_t DOS_LinearToPhysical(void *linear) {
  return reinterpret_cast<uint32_t>(linear) - __djgpp_conventional_base;
}

// Grab a single byte from a segment:offset.
inline uint8_t DOS_Peekuint8_t(const uint32_t segoffset) {
  return _farpeekb(_dos_ds, ((segoffset & 0xFFFF0000) >> 12) + (segoffset & 0xFFFF));
}

// Grab a single 16-bit word from a segment:offset.
inline uint16_t DOS_Peekuint16_t(const uint32_t segoffset) {
  return _farpeekw(_dos_ds, ((segoffset & 0xFFFF0000) >> 12) + (segoffset & 0xFFFF));
}

// Grab a single 32-bit dword from a segment:offset.
inline uint32_t DOS_Peekuint32_t(const uint32_t segoffset) {
  return _farpeekl(_dos_ds, ((segoffset & 0xFFFF0000) >> 12) + (segoffset & 0xFFFF));
}


// Allocate memory under the 640k line; various real mode services and DMA transfers need this.
//  malloc() returns data above 640k because we're a protected mode, 32-bit process, so this is
//  only for specific needs.
void *DOS_AllocateConventionalMemory(int len, _go32_dpmi_seginfo *seginfo);

// Allocate conventional memory suitable for DMA transfers.
void *DOS_AllocateDMAMemory(int len, _go32_dpmi_seginfo *seginfo);

// Free conventional (or DMA, which _is_ conventional) memory.
void DOS_FreeConventionalMemory(_go32_dpmi_seginfo *seginfo);

char *DOS_GetFarPtrCString(uint32_t segoffset);

}
