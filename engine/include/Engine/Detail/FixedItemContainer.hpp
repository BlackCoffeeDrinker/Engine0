
#pragma once

namespace e00 {
template<typename T, std::size_t Capacity>
struct FixedItemContainer {
  using value_type = T;
  using size_type = std::size_t;
  using iterator = T *;
  using const_iterator = const T *;

  std::array<T, Capacity> storage{};
  size_type count{0};

  // --- Capacity & Status ---
  [[nodiscard]] constexpr size_type size() const noexcept { return count; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return Capacity; }
  [[nodiscard]] constexpr bool empty() const noexcept { return count == 0; }
  [[nodiscard]] constexpr bool full() const noexcept { return count == Capacity; }

  // --- Iterators ---
  constexpr iterator begin() noexcept { return storage.data(); }
  constexpr const_iterator begin() const noexcept { return storage.data(); }
  constexpr const_iterator cbegin() const noexcept { return storage.data(); }

  constexpr iterator end() noexcept { return storage.data() + count; }
  constexpr const_iterator end() const noexcept { return storage.data() + count; }
  constexpr const_iterator cend() const noexcept { return storage.data() + count; }

  // --- Element Access ---
  constexpr T &operator[](size_type idx) noexcept { return storage[idx]; }
  constexpr const T &operator[](size_type idx) const noexcept { return storage[idx]; }

  // --- Searching ---

  /// Finds the first slot where operator bool() returns false.
  /// Searches up to Capacity so it works for both sparse arrays and dense vectors.
  constexpr iterator find_first_free() noexcept {
    for (size_type i = 0; i < Capacity; ++i) {
      if (!static_cast<bool>(storage[i])) {
        return storage.data() + i;
      }
    }
    return end();
  }

  constexpr const_iterator find_first_free() const noexcept {
    for (size_type i = 0; i < Capacity; ++i) {
      if (!static_cast<bool>(storage[i])) {
        return storage.data() + i;
      }
    }
    return end();
  }

  /// Finds the first active element matching 'value' via operator==
  template<typename U>
  constexpr iterator find(const U &value) noexcept {
    for (iterator it = begin(); it != end(); ++it) {
      if (static_cast<bool>(*it) && *it == value) {
        return it;
      }
    }
    return end();
  }

  template<typename U>
  constexpr const_iterator find(const U &value) const noexcept {
    for (const_iterator it = begin(); it != end(); ++it) {
      if (static_cast<bool>(*it) && *it == value) {
        return it;
      }
    }
    return end();
  }

  /// Finds the first active element matching a predicate or comparator
  template<typename Predicate>
  constexpr iterator find_if(Predicate pred) noexcept {
    for (iterator it = begin(); it != end(); ++it) {
      if (static_cast<bool>(*it) && pred(*it)) {
        return it;
      }
    }
    return end();
  }

  template<typename Predicate>
  constexpr const_iterator find_if(Predicate pred) const noexcept {
    for (const_iterator it = begin(); it != end(); ++it) {
      if (static_cast<bool>(*it) && pred(*it)) {
        return it;
      }
    }
    return end();
  }

  /// Binary search for sorted arrays using operator< and operator==
  template<typename U>
  constexpr iterator find_sorted(const U &value) noexcept {
    iterator first = begin();
    size_type len = count;
    while (len > 0) {
      size_type half = len / 2;
      iterator middle = first + half;
      if (*middle < value) {
        first = middle + 1;
        len = len - half - 1;
      } else {
        len = half;
      }
    }
    if (first != end() && *first == value) {
      return first;
    }
    return end();
  }

  // --- Vector Operations ---

  constexpr bool push_back(const T &val) noexcept {
    if (full()) return false;
    storage[count++] = val;
    return true;
  }

  constexpr bool push_back(T &&val) noexcept {
    if (full()) return false;
    storage[count++] = std::move(val);
    return true;
  }

  template<typename... Args>
  constexpr T *emplace_back(Args &&...args) noexcept {
    if (full()) return nullptr;
    storage[count] = T{std::forward<Args>(args)...};
    return &storage[count++];
  }

  /// Inserts at the specified iterator, shifting remaining elements right like std::vector
  constexpr iterator insert(const_iterator pos, const T &val) noexcept {
    auto p = const_cast<iterator>(pos);
    if (full() || p < begin() || p > end()) return end();

    for (iterator it = end(); it > p; --it) {
      *it = std::move(*(it - 1));
    }
    *p = val;
    ++count;
    return p;
  }

  /// Inserts item into the first free slot (!item) or appends if no interior free slot exists
  constexpr iterator insert_first_free(T val) noexcept {
    iterator free_slot = find_first_free();
    if (free_slot != storage.data() + Capacity) {
      *free_slot = std::move(val);
      // Expand dense count if insertion happened at/beyond count
      if (const auto idx = static_cast<size_type>(free_slot - storage.data());
          idx >= count) {
        count = idx + 1;
      }
      return free_slot;
    }
    return end();
  }

  /// Erases element at pos and shifts subsequent elements left like std::vector
  constexpr iterator erase(const_iterator pos) noexcept {
    auto it = const_cast<iterator>(pos);
    if (it < begin() || it >= end()) return end();

    for (iterator p = it; p < end() - 1; ++p) {
      *p = std::move(*(p + 1));
    }
    --count;
    storage[count] = T{};// Reset trailing element
    return it;
  }

  /// Erases matching element via operator== with shift
  template<typename U>
  constexpr bool erase_value(const U &val) noexcept {
    auto it = find(val);
    if (it != end()) {
      erase(it);
      return true;
    }
    return false;
  }

  /// Clears by resetting storage and count
  constexpr void clear() noexcept {
    for (size_type i = 0; i < count; ++i) {
      storage[i] = T{};
    }
    count = 0;
  }
};
}// namespace e00
