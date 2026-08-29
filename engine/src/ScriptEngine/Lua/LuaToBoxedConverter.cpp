#include <Engine.hpp>

#include "LuaToBoxedConverter.hpp"
#include "RefFunction.hpp"
#include "UserDataHolder.hpp"


using namespace e00::scripting;
using namespace e00;

namespace {
Property IntegerToX(Property &&original, const TypeInfo &target) {
  auto original_value = cast<lua_Integer>(original);
  if (target == user_type<char>()) return Property((char)original_value);
  if (target == user_type<unsigned char>()) return Property((unsigned char)original_value);
  if (target == user_type<int>()) return Property((int)original_value);
  if (target == user_type<unsigned int>()) return Property((unsigned int)original_value);
  if (target == user_type<long>()) return Property((long)original_value);
  if (target == user_type<unsigned long>()) return Property((unsigned long)original_value);
  if (target == user_type<float>()) return Property((float)original_value);
  if (target == user_type<double>()) return Property((double)original_value);
  if (target == user_type<std::string>()) return Property(std::to_string(original_value));
  return std::move(original);
}

Property FloatingToX(Property &&original, const TypeInfo &target) {
  auto original_value = cast<lua_Number>(original);
  if (target == user_type<char>()) return Property((char)original_value);
  if (target == user_type<unsigned char>()) return Property((unsigned char)original_value);
  if (target == user_type<int>()) return Property((int)original_value);
  if (target == user_type<unsigned int>()) return Property((unsigned int)original_value);
  if (target == user_type<long>()) return Property((long)original_value);
  if (target == user_type<unsigned long>()) return Property((unsigned long)original_value);
  if (target == user_type<float>()) return Property((float)original_value);
  if (target == user_type<double>()) return Property((double)original_value);
  if (target == user_type<std::string>()) return Property(std::to_string(original_value));
  return std::move(original);
}

Property StringToX(Property &&original, const TypeInfo &target) {
  auto original_value = cast<std::string>(original);
  (void)target;
  return std::move(original);
}

Property BoolToX(Property &&original, const TypeInfo &target) {
  bool original_value = cast<bool>(original);
  if (target == user_type<char>()) return Property((char)original_value);
  if (target == user_type<unsigned char>()) return Property((unsigned char)original_value);
  if (target == user_type<int>()) return Property((int)original_value);
  if (target == user_type<unsigned int>()) return Property((unsigned int)original_value);
  if (target == user_type<long>()) return Property((long)original_value);
  if (target == user_type<unsigned long>()) return Property((unsigned long)original_value);
  if (target == user_type<std::string>()) return Property(std::string(original_value ? "true" : "false"));
  return std::move(original);
}

Property NullToX(Property &&original, const TypeInfo &target) {
  if (target.is_pointer()) return Property(nullptr, target);
  if (target == user_type<int>()) return Property(0);
  if (target == user_type<long>()) return Property(0L);
  if (target == user_type<char>()) return Property((char)0);
  if (target == user_type<float>()) return Property(0.f);
  if (target == user_type<double>()) return Property(0.);
  return std::move(original);
}
}// namespace

Property lua_to_boxed_value(lua_State *L, int n, const TypeInfo &info) {
  auto guessed = lua_to_boxed_value_guess(L, n);

  // Shortcut if it's the right contained_type
  if (guessed.get_type_info() == info || guessed.get_type_info().bare_equal_type_info(info)) {
    return guessed;
  }

  constexpr auto luaIntegerType = user_type<lua_Integer>();
  constexpr auto luaNumberType = user_type<lua_Number>();
  constexpr auto luaStringType = user_type<std::string>();
  constexpr auto luaBoolType = user_type<bool>();
  
  // We need to do some conversion
  if (guessed.get_type_info() == luaIntegerType) {
    return IntegerToX(std::move(guessed), info);
  }
  if (guessed.get_type_info() == luaNumberType) {
    return FloatingToX(std::move(guessed), info);
  }
  if (guessed.get_type_info() == luaStringType) {
    return StringToX(std::move(guessed), info);
  }
  if (guessed.get_type_info() == luaBoolType) {
    return BoolToX(std::move(guessed), info);
  }
  if (guessed.get_type_info() == user_type<std::nullptr_t>()) {
    return NullToX(std::move(guessed), info);
  }

  // We found nothing :(
  return guessed;
}

Property lua_to_boxed_value_guess(lua_State *L, int n) {
  switch (lua_type(L, n)) {
    case LUA_TNIL:
      return Property(nullptr);

    case LUA_TNUMBER:
      {
        int isint = 0;
        lua_Integer xi = lua_tointegerx(L, n, &isint);
        if (isint) {
          return Property(xi);
        }

        lua_Number xn = lua_tonumberx(L, n, nullptr);
        return Property(xn);
      }

    case LUA_TBOOLEAN:
      return Property(static_cast<bool>(lua_toboolean(L, n)));

    case LUA_TSTRING:
      {
        size_t len;
        const auto rawstr = lua_tolstring(L, n, &len);
        return Property(std::string(rawstr, len));
      }

    case LUA_TTABLE:
      // a lua map
      break;

    case LUA_TFUNCTION:
      // build a proxy function
      return Property(static_cast<ProxyFunction *>(new lua::RefFunction(L, luaL_ref(L, LUA_REGISTRYINDEX))));

    case LUA_TUSERDATA:
      {
        auto **data = static_cast<lua::UserDataHolder **>(luaL_checkudata(L, n, lua::UserDataHolder::MetaTableName));
        if (data && *data && (*data)->valid()) {
          return (*data)->BoxedData();
        }
      }
      break;

    case LUA_TTHREAD:
      break;

    case LUA_TLIGHTUSERDATA:
      break;
  }

  return Property();
}
