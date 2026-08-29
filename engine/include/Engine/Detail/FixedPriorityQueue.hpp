
#pragma once
#include <array>
#include <cstddef>
#include <utility>

namespace e00 {
template<typename T>
struct FixedLess {
  constexpr bool operator()(const T &lhs, const T &rhs) const noexcept {
    return lhs < rhs;
  }
};

template<typename T, std::size_t Capacity, typename Compare = FixedLess<T>>
class FixedPriorityQueue {
public:
  using value_type = T;
  using size_type = std::size_t;
  using reference = T &;
  using const_reference = const T &;

private:
  std::array<T, Capacity> storage_{};
  size_type size_{0};
  Compare comp_{};

  constexpr void sift_up(size_type index) noexcept {
    while (index > 0) {
      size_type parent = (index - 1) / 2;
      // Max-heap: swap up if parent is less than child
      if (comp_(storage_[parent], storage_[index])) {
        std::swap(storage_[parent], storage_[index]);
        index = parent;
      } else {
        break;
      }
    }
  }

  constexpr void sift_down(size_type index) noexcept {
    while (true) {
      size_type left = 2 * index + 1;
      size_type right = 2 * index + 2;
      size_type largest = index;

      if (left < size_ && comp_(storage_[largest], storage_[left])) {
        largest = left;
      }
      if (right < size_ && comp_(storage_[largest], storage_[right])) {
        largest = right;
      }

      if (largest != index) {
        std::swap(storage_[index], storage_[largest]);
        index = largest;
      } else {
        break;
      }
    }
  }

public:
  constexpr FixedPriorityQueue() = default;
  constexpr explicit FixedPriorityQueue(const Compare &comp) : comp_(comp) {}

  // --- Capacity ---
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
  [[nodiscard]] constexpr bool full() const noexcept { return size_ == Capacity; }
  [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return Capacity; }

  // --- Element Access ---
  [[nodiscard]] constexpr const_reference top() const noexcept {
    return storage_[0];
  }

  // --- Modifiers ---
  constexpr bool push(const T &value) noexcept {
    if (full()) return false;
    storage_[size_] = value;
    sift_up(size_);
    ++size_;
    return true;
  }

  constexpr bool push(T &&value) noexcept {
    if (full()) return false;
    storage_[size_] = std::move(value);
    sift_up(size_);
    ++size_;
    return true;
  }

  template<typename... Args>
  constexpr bool emplace(Args &&...args) noexcept {
    if (full()) return false;
    storage_[size_] = T{std::forward<Args>(args)...};
    sift_up(size_);
    ++size_;
    return true;
  }

  constexpr bool pop() noexcept {
    if (empty()) return false;
    --size_;
    if (size_ > 0) {
      storage_[0] = std::move(storage_[size_]);
      storage_[size_] = T{};// Clear the vacated trailing element
      sift_down(0);
    } else {
      storage_[0] = T{};
    }
    return true;
  }

  constexpr void clear() noexcept {
    for (size_type i = 0; i < size_; ++i) {
      storage_[i] = T{};
    }
    size_ = 0;
  }
};
}// namespace e00
