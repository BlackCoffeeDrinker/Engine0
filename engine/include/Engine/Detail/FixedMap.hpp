
#pragma once

namespace e00 {
#include <array>
#include <cstddef>
#include <utility>

template<typename Key, typename Value, std::size_t Capacity>
class FixedMap {
public:
  using key_type = Key;
  using mapped_type = Value;

  // NOTE: a plain aggregate, not `std::pair<Key, Value>`. `std::pair`'s default constructor is
  // gated behind `requires (is_default_constructible_v<T1>) && (is_default_constructible_v<T2>)`,
  // and on mingw-w64's GCC 16.2 libstdc++, that trait can spuriously evaluate to `false` for a
  // `Value` type with an `auto`-deduced-return-type member function (e.g. `e00::World::Element`),
  // even though directly default-constructing that same type works fine everywhere. Using our own
  // aggregate sidesteps `std::pair` (and its buggy constrained default constructor) entirely: an
  // aggregate's members are just default-initialized member-by-member, no trait gating involved.
  // `.first`/`.second` naming is kept so the rest of this class (and callers) don't need to change,
  // and it still supports structured bindings like `std::pair` does.
  struct value_type {
    Key first{};
    Value second{};
  };
  using size_type = std::size_t;
  using iterator = value_type *;
  using const_iterator = const value_type *;

private:
  std::array<value_type, Capacity> data_{};
  size_type size_{0};

  // Freestanding lower_bound implementation (binary search)
  constexpr const_iterator find_lower_bound(const Key &key) const noexcept {
    const_iterator first = data_.data();
    size_type count = size_;
    while (count > 0) {
      size_type step = count / 2;
      const_iterator it = first + step;
      if (it->first < key) {
        first = ++it;
        count -= step + 1;
      } else {
        count = step;
      }
    }
    return first;
  }

  constexpr iterator find_lower_bound(const Key &key) noexcept {
    return const_cast<iterator>(static_cast<const FixedMap *>(this)->find_lower_bound(key));
  }

public:
  constexpr FixedMap() = default;

  // Capacity
  [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return Capacity; }
  [[nodiscard]] constexpr size_type max_size() const noexcept { return Capacity; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
  [[nodiscard]] constexpr bool full() const noexcept { return size_ == Capacity; }

  // Iterators
  constexpr iterator begin() noexcept { return data_.data(); }
  constexpr const_iterator begin() const noexcept { return data_.data(); }
  constexpr const_iterator cbegin() const noexcept { return data_.data(); }

  constexpr iterator end() noexcept { return data_.data() + size_; }
  constexpr const_iterator end() const noexcept { return data_.data() + size_; }
  constexpr const_iterator cend() const noexcept { return data_.data() + size_; }

  // Lookup - O(log N)
  constexpr iterator lower_bound(const Key &key) noexcept { return find_lower_bound(key); }
  constexpr const_iterator lower_bound(const Key &key) const noexcept { return find_lower_bound(key); }

  constexpr iterator find(const Key &key) noexcept {
    auto it = find_lower_bound(key);
    if (it != end() && !(key < it->first)) {
      return it;
    }
    return end();
  }

  constexpr const_iterator find(const Key &key) const noexcept {
    auto it = find_lower_bound(key);
    if (it != end() && !(key < it->first)) {
      return it;
    }
    return end();
  }

  constexpr bool contains(const Key &key) const noexcept {
    return find(key) != end();
  }

  // Element Access
  constexpr Value *at(const Key &key) noexcept {
    auto it = find(key);
    return (it != end()) ? &it->second : nullptr;
  }

  constexpr const Value *at(const Key &key) const noexcept {
    auto it = find(key);
    return (it != end()) ? &it->second : nullptr;
  }

  constexpr Value &operator[](const Key &key) noexcept {
    auto it = find_lower_bound(key);
    if (it != end() && !(key < it->first)) {
      return it->second;
    }
    if (full()) {
      // Traps in freestanding if capacity is exceeded
      __builtin_trap();
    }
    for (iterator p = end(); p > it; --p) {
      *p = std::move(*(p - 1));
    }
    ++size_;
    *it = value_type{key, Value{}};
    return it->second;
  }

  // Insertion - O(N)
  constexpr std::pair<iterator, bool> insert(const value_type &val) noexcept {
    auto it = find_lower_bound(val.first);
    if (it != end() && !(val.first < it->first)) {
      return {it, false};
    }
    if (full()) {
      return {end(), false};
    }
    for (iterator p = end(); p > it; --p) {
      *p = std::move(*(p - 1));
    }
    *it = val;
    ++size_;
    return {it, true};
  }

  constexpr std::pair<iterator, bool> insert(value_type &&val) noexcept {
    auto it = find_lower_bound(val.first);
    if (it != end() && !(val.first < it->first)) {
      return {it, false};
    }
    if (full()) {
      return {end(), false};
    }
    for (iterator p = end(); p > it; --p) {
      *p = std::move(*(p - 1));
    }
    *it = std::move(std::move(val));
    ++size_;
    return {it, true};
  }

  constexpr std::pair<iterator, bool> insert(const Key &key, Value &&val) noexcept {
    return insert(value_type{key, std::forward<Value>(val)});
  }

  template<typename... Args>
  constexpr std::pair<iterator, bool> try_emplace(const Key &key, Args &&...args) noexcept {
    auto it = find_lower_bound(key);
    if (it != end() && !(key < it->first)) {
      return {it, false};// Key already exists
    }
    if (full()) {
      return {end(), false};// Capacity reached
    }
    for (iterator p = end(); p > it; --p) {
      *p = std::move(*(p - 1));
    }

    // Construct pair in-place
    *it = value_type{key, Value(std::forward<Args>(args)...)};
    ++size_;
    return {it, true};
  }
  
  // Erasure - O(N)
  constexpr iterator erase(const_iterator pos) noexcept {
    iterator it = const_cast<iterator>(pos);
    if (it < begin() || it >= end()) {
      return end();
    }
    for (iterator p = it; p < end() - 1; ++p) {
      *p = std::move(*(p + 1));
    }
    --size_;
    return it;
  }

  constexpr size_type erase(const Key &key) noexcept {
    auto it = find(key);
    if (it != end()) {
      erase(it);
      return 1;
    }
    return 0;
  }

  constexpr void clear() noexcept {
    size_ = 0;
  }
};
}// namespace e00
