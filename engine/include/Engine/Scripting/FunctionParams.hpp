#pragma once

#include <array>
#include <vector>

#include "BoxedValue.hpp"

namespace e00::scripting {
/**
 *
 */
struct FunctionParams {
  constexpr FunctionParams(const BoxedValue *const t_begin, const BoxedValue *const t_end)
    : m_begin(t_begin), m_end(t_end) {
  }

  explicit FunctionParams(const BoxedValue &bv)
    : m_begin(&bv), m_end(m_begin + 1) {
  }

  explicit FunctionParams(const std::vector<BoxedValue> &vec)
    : m_begin(vec.empty() ? nullptr : &vec.front()),
      m_end(vec.empty() ? nullptr : &vec.front() + vec.size()) {
  }

  constexpr FunctionParams() : m_begin(nullptr), m_end(nullptr) {}

  template<size_t Size>
  constexpr explicit FunctionParams(const std::array<BoxedValue, Size> &a)
    : m_begin(a.data()), m_end(a.data() + a.size()) {
  }

  constexpr explicit FunctionParams(const std::array<BoxedValue, 0> &)
    : m_begin(nullptr), m_end(nullptr) {
  }

  [[nodiscard]] constexpr const BoxedValue &operator[](const std::size_t t_i) const noexcept {
    return m_begin[t_i];
  }

  [[nodiscard]] constexpr const BoxedValue *begin() const noexcept {
    return m_begin;
  }

  [[nodiscard]] constexpr const BoxedValue &front() const noexcept {
    return *m_begin;
  }

  [[nodiscard]] constexpr const BoxedValue *end() const noexcept {
    return m_end;
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return static_cast<size_t>(m_end - m_begin);
  }

  std::vector<BoxedValue> to_vector() const {
    return std::vector<BoxedValue>{ m_begin, m_end };
  }

  [[nodiscard]] constexpr bool empty() const noexcept {
    return m_begin == m_end;
  }

private:
  const BoxedValue *m_begin;
  const BoxedValue *m_end;
};
}// namespace e00::scripting
