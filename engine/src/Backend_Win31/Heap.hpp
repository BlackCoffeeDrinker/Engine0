#pragma once

#include "Win32Types.hpp"

#include <cstddef>
#include <new>

namespace win31 {
void *HeapAllocate(std::size_t size);
void *HeapReallocate(void *ptr, std::size_t size);
void HeapDeallocate(void *ptr) noexcept;
}// namespace win31
