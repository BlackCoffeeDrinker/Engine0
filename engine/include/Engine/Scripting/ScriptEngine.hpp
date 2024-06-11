#pragma once

#include <string>
#include <type_traits>
#include <system_error>
#include <utility>
#include <functional>
#include <memory>

#include "detail/NativeFunctionT.hpp"
#include "BoxedCast.hpp"

namespace e00 {
class ScriptEngine {
protected:
  ScriptEngine();

  virtual bool valid_fn_name(const std::string &fn_name) = 0;

  virtual void add_function(const std::string &fn_name, std::unique_ptr<scripting::ProxyFunction> &&fn) = 0;

  virtual void add_variable(const std::string &var_name, scripting::BoxedValue val) = 0;

  virtual void add_type(const scripting::TypeInfo &type) = 0;

public:
  static std::unique_ptr<ScriptEngine> Create();

  virtual ~ScriptEngine();

  virtual const std::string &engine_name() const = 0;

  template<typename Type>
  void register_type() {
    add_type(scripting::user_type<Type>());
  }

  template<typename Fn>
  void register_function(const std::string &fn_name, Fn fn) {
    if (valid_fn_name(fn_name)) {
      auto function_t = scripting::detail::make_function_t(fn, scripting::detail::FunctionSignature{ fn });
      for (const auto &i : function_t->parameters()) {
        add_type(i);
      }
      add_type(function_t->return_type());
      add_function(fn_name, std::move(function_t));
    }
  }

  template<typename VarType>
  void register_variable(const std::string &var_name, VarType var) {
    register_type<VarType>();
    add_variable(var_name, scripting::BoxedValue(var));
  }

  virtual std::unique_ptr<scripting::ProxyFunction> get_function(const std::string &fn_name, scripting::TypeInfo preferred_return_type) = 0;

  template<typename Ret, typename... Args>
  Ret call(const std::string &fn_name, Args &&...args) {
    if (auto fn = get_function(fn_name, scripting::user_type<Ret>())) {
      auto ret = fn->operator()(std::forward<Args>(args)...);

      if constexpr (std::is_void<Ret>::value) {
        return;
      } else {
        return scripting::cast<Ret>(ret);
      }
    }

    // FAILSAFE: ERR OUT
    return Ret();
  }

  // Default implementation reads everything and calls the string version
  virtual std::error_code parse(const std::unique_ptr<e00::Stream> &stream);

  virtual std::error_code parse(const std::string &code) = 0;
};

}// namespace e00
