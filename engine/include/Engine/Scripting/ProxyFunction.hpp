#pragma once

#include <vector>
#include <limits>

#include "TypeInfo.hpp"
#include "BoxedValue.hpp"
#include "FunctionParams.hpp"

namespace e00::scripting {
/**
 * A callable method
 * This is the only function the script engine impl can deal with
 */
class ProxyFunction {
  static const TypeInfo _end;

  const bool _valid;
  const size_t _param_count;
  const std::vector<TypeInfo> _params;
  const TypeInfo _return_type;

protected:
  static constexpr size_t VARIABLE_ARGUMENTS = std::numeric_limits<decltype(_param_count)>::max();

  ProxyFunction(std::vector<TypeInfo> types, TypeInfo return_type, size_t args_cnt)
    : _valid(true),
      _param_count(args_cnt),
      _params(std::move(types)),
      _return_type(return_type) {
  }

  /* Actually do the call */
  [[nodiscard]] virtual BoxedValue do_call(const FunctionParams &params) const = 0;

public:
  ProxyFunction()
    : _valid(false),
      _param_count(0),
      _params({}),
      _return_type(TypeInfo()) {}

  virtual ~ProxyFunction() = default;

  const std::vector<TypeInfo> &parameters() const { return _params; }

  const TypeInfo &parameter(size_t param) const noexcept {
    if (param >= _params.size()) {
      if (is_varargs() && parameter_count() > 0) {
        return _params.back();
      }

      return _end;
    }

    return _params.at(param);
  }

  const TypeInfo &return_type() const { return _return_type; }

  virtual bool is_attribute_function() const noexcept { return false; }

  virtual bool is_member() const noexcept { return false; }

  bool is_varargs() const noexcept { return _param_count == VARIABLE_ARGUMENTS; }

  size_t parameter_count() const noexcept { return _params.size(); }

  inline BoxedValue call(const FunctionParams &params) const {
    if (is_varargs() || size_t(_param_count) == params.size()) {
      return do_call(params);
    }

    return {};
  }

  template<typename... Args>
  inline BoxedValue operator()(Args &&...args) const {
    std::array<BoxedValue, sizeof...(Args)> boxed_args = {
      BoxedValue(std::forward<Args>(args))...,
    };

    return call(FunctionParams(boxed_args));
  }

  operator bool() const { return _valid; }
};

template<>
inline BoxedValue ProxyFunction::operator()() const {
  return call(FunctionParams());
}

}// namespace e00::scripting
