#pragma once

#include <functional>
#include <memory>

#include "../BoxedValue.hpp"
#include "Utils.hpp"
#include "NativeFunctionT.hpp"

namespace e00::scripting::detail {
/// Used internally for handling a return value from a Proxy_Function call
template<typename Ret>
struct Handle_Return {
  template<typename T,
    typename = std::enable_if_t<std::is_pod_v<std::decay_t<T>>>>
  static BoxedValue handle(T r) {
    return BoxedValue(std::move(r), true);
  }

  template<typename T,
    typename = std::enable_if_t<!std::is_pod_v<std::decay_t<T>>>>
  static BoxedValue handle(T &&r) {
    return BoxedValue(std::forward<T>(r), true);
  }
};

// template<typename Ret>
// struct Handle_Return<const std::function<Ret> &> {
//   static BoxedValue handle(const std::function<Ret> &f) {
//     return BoxedValue(
//       make_shared_base<ProxyFunction, NativeFunctionT<Ret, std::function<Ret>>>(f));
//   }
// };
//
// template<typename Ret>
// struct Handle_Return<std::function<Ret>> : Handle_Return<const std::function<Ret> &> {
// };

// template<typename Ret>
// struct Handle_Return<const std::shared_ptr<std::function<Ret>>> {
//   static BoxedValue handle(const std::shared_ptr<std::function<Ret>> &f) {
//     return BoxedValue(
//       make_shared_base<ProxyFunction, dispatch::Assignable_Proxy_Function_Impl<Ret>>(std::ref(*f), f));
//   }
// };

// template<typename Ret>
// struct Handle_Return<const std::shared_ptr<std::function<Ret>> &> : Handle_Return<const std::shared_ptr<std::function<Ret>>> {
// };
//
// template<typename Ret>
// struct Handle_Return<std::shared_ptr<std::function<Ret>>> : Handle_Return<const std::shared_ptr<std::function<Ret>>> {
// };

// template<typename Ret>
// struct Handle_Return<std::function<Ret> &> {
//   static BoxedValue handle(std::function<Ret> &f) {
//     return BoxedValue(
//       make_shared_base<ProxyFunction, dispatch::Assignable_Proxy_Function_Impl<Ret>>(std::ref(f),
//         std::shared_ptr<std::function<Ret>>()));
//   }
//
//   static BoxedValue handle(const std::function<Ret> &f) {
//     return BoxedValue(
//       make_shared_base<ProxyFunction, NativeFunctionT<Ret, std::function<Ret>>>(f));
//   }
// };

template<typename Ret>
struct Handle_Return<Ret *&> {
  static BoxedValue handle(Ret *p) {
    return BoxedValue(p, true);
  }
};

template<typename Ret>
struct Handle_Return<const Ret *&> {
  static BoxedValue handle(const Ret *p) {
    return BoxedValue(p, true);
  }
};

template<typename Ret>
struct Handle_Return<Ret *> {
  static BoxedValue handle(Ret *p) {
    return BoxedValue(p, true);
  }
};

template<typename Ret>
struct Handle_Return<const Ret *> {
  static BoxedValue handle(const Ret *p) {
    return BoxedValue(p, true);
  }
};

template<typename Ret>
struct Handle_Return<std::shared_ptr<Ret> &> {
  static BoxedValue handle(const std::shared_ptr<Ret> &r) {
    return BoxedValue(r, true);
  }
};

template<typename Ret>
struct Handle_Return<std::shared_ptr<Ret>> : Handle_Return<std::shared_ptr<Ret> &> {
};

template<typename Ret>
struct Handle_Return<const std::shared_ptr<Ret> &> : Handle_Return<std::shared_ptr<Ret> &> {
};


template<typename Ret>
struct Handle_Return<std::unique_ptr<Ret>> : Handle_Return<std::unique_ptr<Ret> &> {
  static BoxedValue handle(std::unique_ptr<Ret> &&r) {
    return BoxedValue(std::move(r), true);
  }
};

template<typename Ret, bool Ptr>
struct Handle_Return_Ref {
  template<typename T>
  static BoxedValue handle(T &&r) {
    return BoxedValue(std::cref(r), true);
  }
};

template<typename Ret>
struct Handle_Return_Ref<Ret, true> {
  template<typename T>
  static BoxedValue handle(T &&r) {
    return BoxedValue(typename std::remove_reference<decltype(r)>::type{ r }, true);
  }
};

template<typename Ret>
struct Handle_Return<const Ret &> : Handle_Return_Ref<const Ret &, std::is_pointer<typename std::remove_reference<const Ret &>::type>::value> {
};


template<typename Ret>
struct Handle_Return<const Ret> {
  static BoxedValue handle(Ret r) {
    return BoxedValue(std::move(r));
  }
};

template<typename Ret>
struct Handle_Return<Ret &> {
  static BoxedValue handle(Ret &r) {
    return BoxedValue(std::ref(r));
  }
};

template<>
struct Handle_Return<BoxedValue> {
  static BoxedValue handle(const BoxedValue &r) noexcept {
    return r;
  }
};

template<>
struct Handle_Return<const BoxedValue> : Handle_Return<BoxedValue> {
};

template<>
struct Handle_Return<BoxedValue &> : Handle_Return<BoxedValue> {
};

template<>
struct Handle_Return<const BoxedValue &> : Handle_Return<BoxedValue> {
};


/**
 * Used internally for handling a return value from a Proxy_Function call
 */
template<>
struct Handle_Return<void> {
  static BoxedValue handle() {
    return void_var();
  }
};
}// namespace e00::scripting::detail
