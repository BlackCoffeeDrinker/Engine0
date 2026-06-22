
#include "memory.hpp"

#include <stdlib.h>

void *DOS_AllocateConventionalMemory(const int len, _go32_dpmi_seginfo *seginfo) {
  seginfo->size = (len + 15) / 16;// this is in "paragraphs"
  if (_go32_dpmi_allocate_dos_memory(seginfo) != 0) {
    exit(200);
    return nullptr;
  }
  return DOS_PhysicalToLinear(seginfo->rm_segment * 16);
}

void *DOS_AllocateDMAMemory(const int len, _go32_dpmi_seginfo *seginfo) {
  // ISA DMA transfers cannot cross a 64 KB physical page boundary (hardware
  // limitation of the 8237 DMA controller). Allocating 2× the requested size
  // guarantees at least one contiguous `len`-byte region that doesn't straddle
  // a boundary. This is the standard technique used by Allegro, MIDAS, and
  // every other DOS audio library; allocate-check-retry would add complexity
  // for zero benefit.
  auto *ptr = static_cast<uint8_t *>(DOS_AllocateConventionalMemory(len * 2, seginfo));
  if (!ptr) {
    return nullptr;
  }

  // if we're past the end of a page, use the second half of the block.
  const uint32_t physical = (seginfo->rm_segment * 16);
  if ((physical >> 16) != ((physical + len) >> 16)) {
    ptr += len;
  }
  return ptr;
}

void DOS_FreeConventionalMemory(_go32_dpmi_seginfo *seginfo) {
  _go32_dpmi_free_dos_memory(seginfo);
}

char *DOS_GetFarPtrCString(const uint32_t segoffset) {
  if (!segoffset) {// let's just treat this as a NULL pointer.
    return nullptr;
  }

  const auto ofs = (unsigned long) (((segoffset & 0xFFFF0000) >> 12) + (segoffset & 0xFFFF));
  size_t len;

  for (len = 0; _farpeekb(_dos_ds, ofs + len) != '\0'; len++) {
  }

  len++;// null terminator.
  const auto retval = static_cast<char *>(malloc(len));
  if (!retval) {
    return nullptr;
  }

  for (size_t i = 0; i < len; i++) {
    retval[i] = static_cast<char>(_farpeekb(_dos_ds, ofs + i));
  }
  return retval;
}

