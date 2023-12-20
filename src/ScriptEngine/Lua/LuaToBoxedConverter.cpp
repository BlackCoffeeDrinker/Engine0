#include "LuaToBoxedConverter.hpp"
#include "RefFunction.hpp"
#include "UserDataHolder.hpp"

#include <Engine/Scripting/BoxedCast.hpp>

using namespace e00::scripting;

namespace {
BoxedValue IntegerToX(BoxedValue &&original, const TypeInfo &target) {
  auto original_value = cast<lua_Integer>(original);
  if (target == user_type<char>()) return BoxedValue((char)original_value);
  if (target == user_type<unsigned char>()) return BoxedValue((unsigned char)original_value);
  if (target == user_type<int>()) return BoxedValue((int)original_value);
  if (target == user_type<unsigned int>()) return BoxedValue((unsigned int)original_value);
  if (target == user_type<long>()) return BoxedValue((long)original_value);
  if (target == user_type<unsigned long>()) return BoxedValue((unsigned long)original_value);
  if (target == user_type<float>()) return BoxedValue((float)original_value);
  if (target == user_type<double>()) return BoxedValue((double)original_value);
  if (target == user_type<std::string>()) return BoxedValue(std::to_string(original_value));
  return std::move(original);
}

BoxedValue FloatingToX(BoxedValue &&original, const TypeInfo &target) {
  auto original_value = cast<lua_Number>(original);
  if (target == user_type<char>()) return BoxedValue((char)original_value);
  if (target == user_type<unsigned char>()) return BoxedValue((unsigned char)original_value);
  if (target == user_type<int>()) return BoxedValue((int)original_value);
  if (target == user_type<unsigned int>()) return BoxedValue((unsigned int)original_value);
  if (target == user_type<long>()) return BoxedValue((long)original_value);
  if (target == user_type<unsigned long>()) return BoxedValue((unsigned long)original_value);
  if (target == user_type<float>()) return BoxedValue((float)original_value);
  if (target == user_type<double>()) return BoxedValue((double)original_value);
  if (target == user_type<std::string>()) return BoxedValue(std::to_string(original_value));
  return std::move(original);
}

BoxedValue StringToX(BoxedValue &&original, const TypeInfo &target) {
  auto original_value = cast<std::string>(original);
  (void)target;
  return std::move(original);
}

BoxedValue BoolToX(BoxedValue &&original, const TypeInfo &target) {
  bool original_value = cast<bool>(original);
  if (target == user_type<char>()) return BoxedValue((char)original_value);
  if (target == user_type<unsigned char>()) return BoxedValue((unsigned char)original_value);
  if (target == user_type<int>()) return BoxedValue((int)original_value);
  if (target == user_type<unsigned int>()) return BoxedValue((unsigned int)original_value);
  if (target == user_type<long>()) return BoxedValue((long)original_value);
  if (target == user_type<unsigned long>()) return BoxedValue((unsigned long)original_value);
  if (target == user_type<std::string>()) return BoxedValue(std::string(original_value ? "true" : "false"));
  return std::move(original);
}

BoxedValue NullToX(BoxedValue &&original, const TypeInfo &target) {
  if (target.is_pointer()) return BoxedValue(nullptr, target);
  if (target == user_type<int>()) return BoxedValue((int)0);
  if (target == user_type<long>()) return BoxedValue(0l);
  if (target == user_type<char>()) return BoxedValue((char)0);
  if (target == user_type<float>()) return BoxedValue(0.f);
  if (target == user_type<double>()) return BoxedValue((double)0.);
  return std::move(original);
}
}// namespace

BoxedValue lua_to_boxed_value(lua_State *L, int n, const TypeInfo &info) {
  auto guessed = lua_to_boxed_value_guess(L, n);

  // Shortcut if it's the right contained_type
  if (guessed.get_type_info() == info || guessed.get_type_info().bare_equal_type_info(info)) {
    return guessed;
  }

  // We need to do some conversion
  if (guessed.get_type_info() == user_type<lua_Integer>()) {
    return IntegerToX(std::move(guessed), info);
  } else if (guessed.get_type_info() == user_type<lua_Number>()) {
    return FloatingToX(std::move(guessed), info);
  } else if (guessed.get_type_info() == user_type<std::string>()) {
    return StringToX(std::move(guessed), info);
  } else if (guessed.get_type_info() == user_type<bool>()) {
    return BoolToX(std::move(guessed), info);
  } else if (guessed.get_type_info() == user_type<nullptr_t>()) {
    return NullToX(std::move(guessed), info);
  }

  // We found nothing :(
  return guessed;
}

BoxedValue lua_to_boxed_value_guess(lua_State *L, int n) {
  switch (lua_type(L, n)) {
    case LUA_TNIL:
      return BoxedValue(nullptr);

    case LUA_TNUMBER:
      {
        int isint = 0;
        lua_Integer xi = lua_tointegerx(L, n, &isint);
        if (isint) {
          return BoxedValue(xi);
        }

        lua_Number xn = lua_tonumberx(L, n, nullptr);
        return BoxedValue(xn);
      }

    case LUA_TBOOLEAN:
      return BoxedValue(static_cast<bool>(lua_toboolean(L, n)));

    case LUA_TSTRING:
      {
        size_t len;
        const auto rawstr = lua_tolstring(L, n, &len);
        return BoxedValue(std::string(rawstr, len));
      }

    case LUA_TTABLE:
      // a lua map
      break;

    case LUA_TFUNCTION:
      // build a proxy function
      return BoxedValue((ProxyFunction *)new lua::RefFunction(L, luaL_ref(L, LUA_REGISTRYINDEX)));

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

  return BoxedValue();
}
