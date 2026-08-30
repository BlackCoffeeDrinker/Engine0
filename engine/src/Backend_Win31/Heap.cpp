#include "Heap.hpp"

namespace win31 {
void *HeapAllocate(std::size_t size) {
  if (size == 0) {
    size = 1;
  }
  HANDLE heap = GetProcessHeap();
  if (!heap) {
    MessageBoxA(nullptr, "HeapAllocate", "Error", 0x00000000L);
    return nullptr;
  }
  return HeapAlloc(heap, 0, static_cast<DWORD>(size));
}

void *HeapReallocate(void *ptr, std::size_t size) {
  if (!ptr) {
    return HeapAllocate(size);
  }
  if (size == 0) {
    HeapDeallocate(ptr);
    return nullptr;
  }
  HANDLE heap = GetProcessHeap();
  if (!heap) {
    return nullptr;
  }
  return HeapReAlloc(heap, 0, ptr, static_cast<DWORD>(size));
}

void HeapDeallocate(void *ptr) noexcept {
  if (!ptr) {
    return;
  }
  HANDLE heap = GetProcessHeap();
  if (heap) {
    HeapFree(heap, 0, ptr);
  }
}
}// namespace win31

void *operator new(std::size_t size) {
  void *p = win31::HeapAllocate(size);
  // Under -fno-exceptions we cannot throw std::bad_alloc; return null and hope callers check,
  // or abort. Prefer abort for hard OOM on this tiny target.
  if (!p) {
    MessageBoxA(nullptr, "NEW FAILED", "Error", 0x00000000L);
    ExitProcess(1);
  }
  return p;
}

void *operator new[](std::size_t size) {
  return operator new(size);
}

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
  return win31::HeapAllocate(size);
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept {
  return win31::HeapAllocate(size);
}

void operator delete(void *ptr) noexcept {
  win31::HeapDeallocate(ptr);
}

void operator delete[](void *ptr) noexcept {
  win31::HeapDeallocate(ptr);
}

void operator delete(void *ptr, std::size_t) noexcept {
  win31::HeapDeallocate(ptr);
}

void operator delete[](void *ptr, std::size_t) noexcept {
  win31::HeapDeallocate(ptr);
}

void operator delete(void *ptr, const std::nothrow_t &) noexcept {
  win31::HeapDeallocate(ptr);
}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept {
  win31::HeapDeallocate(ptr);
}

// Provide malloc/free for C vendors (zlib/lua/lodepng) without relying solely on CRT.
extern "C" {

void *malloc(size_t size) {
  return win31::HeapAllocate(size);
}

void *calloc(size_t n, size_t size) {
  const size_t total = n * size;
  void *p = win31::HeapAllocate(total);
  if (p) {
    // memset may come from Intrinsics.cpp or CRT
    unsigned char *b = static_cast<unsigned char *>(p);
    for (size_t i = 0; i < total; ++i) b[i] = 0;
  }
  return p;
}

void *realloc(void *ptr, size_t size) {
  return win31::HeapReallocate(ptr, size);
}

void free(void *ptr) {
  win31::HeapDeallocate(ptr);
}
}
