#pragma once

#include <memory>

#include "TypeInfo.hpp"
#include "detail/FunctionSignature.hpp"

namespace e00::scripting {
/**
 * Wrapper for holding any support C++ contained_type
 * The script engine only understands BoxedValues for values
 */
struct BoxedValue {
public:
  /// used for explicitly creating a "void" object
  struct VoidType {};

private:
  struct Data {
  protected:
    Data() = default;

  public:
    Data &operator=(const Data &) = delete;
    virtual ~Data() noexcept = default;
    virtual void *data() noexcept = 0;
    [[nodiscard]] virtual std::unique_ptr<Data> clone() const = 0;
  };

  struct DataCtrl {
    int refs;
    DataCtrl() : refs(0) {}
  };

  template<typename T>
  struct DataImplOwned : Data {
    DataCtrl *_ctrl;
    T _data;

    explicit DataImplOwned(T mdata)
      : _ctrl(new DataCtrl()),
        _data(std::move(mdata)) {
      _ctrl->refs++;
    }

    DataImplOwned(T mdata, DataCtrl *mctrl)
      : _ctrl(mctrl),
        _data(mdata) {
      _ctrl->refs++;
    }

    ~DataImplOwned() override {
      _ctrl->refs--;
      if (_ctrl->refs <= 0) {
        delete _ctrl;
        delete _data;
      }
      _ctrl = nullptr;
      _data = nullptr;
    }

    void *data() noexcept override { return _data; }
    [[nodiscard]] std::unique_ptr<Data> clone() const override {
      return std::make_unique<DataImplOwned<T>>(_data, _ctrl);
    }
    DataImplOwned &operator=(const DataImplOwned &) = delete;
  };

  template<typename T>
  struct DataImplPtr : Data {
    T _data;

    explicit DataImplPtr(T mdata) : _data(std::move(mdata)) {}
    ~DataImplPtr() override {}
    void *data() noexcept override { return _data; }
    [[nodiscard]] std::unique_ptr<Data> clone() const override {
      return std::make_unique<DataImplPtr<T>>(_data);
    }
    DataImplPtr &operator=(const DataImplPtr &) = delete;
  };

  struct DataBuilder {
    template<typename T>
    constexpr static bool is_const() {
      return std::is_const<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value;
    }

    template<typename T>
    static std::unique_ptr<Data> get(T *t) {
      return std::make_unique<DataImplPtr<std::decay_t<decltype(t)>>>(std::forward<decltype(t)>(t));
    }

    template<typename T>
    static std::unique_ptr<Data> get(const T *t) {
      return std::make_unique<DataImplPtr<std::decay_t<decltype(t)>>>(std::forward<decltype(t)>(t));
    }

    template<typename T>
    static std::unique_ptr<Data> get(T t) {
      auto p = new T(std::move(t));
      return std::make_unique<DataImplOwned<std::decay_t<decltype(p)>>>(std::forward<decltype(p)>(p));
    }
  };

public:
  /// Basic Boxed_Value constructor
  template<typename T, typename = std::enable_if_t<!std::is_same_v<BoxedValue, std::decay_t<T>>>>
  explicit BoxedValue(T &&t, bool t_return_value = false)
    : _info(user_type<T>()),
      _is_return(t_return_value),
      _data(DataBuilder::get(std::forward<T>(t))) {
  }

  // Report as another contained_type (for pointers)
  template<typename T, typename = std::enable_if_t<!std::is_same_v<BoxedValue, std::decay_t<T>>>>
  explicit BoxedValue(T &&t, TypeInfo type, bool t_return_value = false)
    : _info(type),
      _is_return(t_return_value),
      _data(DataBuilder::get(std::forward<T>(t))) {
  }

  // Unknown-contained_type constructor
  BoxedValue() : _info(TypeInfo()), _is_return(false), _data() {}

  BoxedValue(BoxedValue &&other) = default;

  BoxedValue &operator=(BoxedValue &&rhs) = default;

  BoxedValue(const BoxedValue &other)
    : _info(other._info),
      _is_return(other._is_return),
      _data(other._data->clone()) {}

  BoxedValue &operator=(const BoxedValue &rhs) {
    if (&rhs != this) {
      _data = rhs._data->clone();
      _info = rhs._info;
      _is_return = rhs._is_return;
    }
    return *this;
  }

  ~BoxedValue() = default;

  const TypeInfo &get_type_info() const { return _info; }

  bool is_const() const noexcept { return _info.is_const(); }
  bool is_reference() const noexcept { return _info.is_reference(); }
  bool is_void() const noexcept { return _info.is_void(); }
  bool is_arithmetic() const noexcept { return _info.is_arithmetic(); }
  bool is_undef() const noexcept { return _info.is_undef(); }
  bool is_pointer() const noexcept { return _info.is_pointer(); }
  bool is_class() const noexcept { return _info.is_class(); }

  const void *get_const_ptr() const noexcept { return (_data) ? _data->data() : nullptr; }
  void *get_ptr() const noexcept { return (_data) ? _data->data() : nullptr; }

private:
  TypeInfo _info;
  bool _is_return;
  std::unique_ptr<Data> _data;
};

inline BoxedValue void_var() {
  return BoxedValue(BoxedValue::VoidType());
}
}// namespace e00::scripting
