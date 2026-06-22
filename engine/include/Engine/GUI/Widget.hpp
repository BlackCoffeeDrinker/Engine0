#pragma once

namespace e00 {
class Painter;

/**
 * @class Widget
 * @brief Represents a general-purpose interface component or element.
 *
 * The Widget class serves as a base representation for UI elements or
 * application components, encapsulating properties and behavior that
 * can be extended or specialized for specific use-cases.
 */
class Widget {
public:
  using WidgetPtrT = Widget *;
  using OwnedWidgetPtrT = std::unique_ptr<Widget>;

  enum class BackgroundType {
    None,
    Solid,
    Image,
  };

  enum class ActionProcessResult {
    NotHandled,
    Handled,
    HandledAndConsumed,
  };

private:
  using SIZE_TYPE = uint16_t;

  std::string _name;
  WidgetPtrT _parent;
  std::vector<OwnedWidgetPtrT> _children;

  // Sizing
  RectT<SIZE_TYPE> _rect;
  bool _has_min_set = false;
  bool _has_max_set = false;
  Vec2D<SIZE_TYPE> _min;
  Vec2D<SIZE_TYPE> _max;

  Vec2D<SIZE_TYPE> _computed_size;
  bool _has_computed_size = false;

  // Focus
  bool _can_have_focus = false;
  bool _has_focus = false;

  // Events
  bool _can_process_actions = true;

  // Background
  BackgroundType _background_type = BackgroundType::None;
  Color _background_color;

protected:
  virtual void ResizeEvent() {
    _has_computed_size = false;
    for (auto &child: _children) {
      child->ResizeEvent();
    }
  }

  virtual void ComputeSize() {
    _computed_size = _min;
    _has_computed_size = true;
  }

  void SetComputedSize(const Vec2D<SIZE_TYPE> &size) {
    _computed_size = size;
    _has_computed_size = true;
  }

public:
  Widget();
  explicit Widget(std::string name);
  virtual ~Widget();

  void SetName(std::string &&name) { _name = std::move(name); }
  [[nodiscard]] const auto &Name() const { return _name; }

  /**
   * @brief Adds an owned child widget.
   *
   * Ownership of the child is transferred to this widget. The returned
   * reference remains valid until the child is removed or this widget is
   * destroyed.
   *
   * @param child The widget to add.
   * @return Reference to the added child.
   */
  Widget *AddChild(OwnedWidgetPtrT child);

  /**
   * @brief Constructs and adds an owned child widget.
   *
   * @tparam T Widget-derived type to construct.
   * @tparam Args Constructor argument types.
   * @param args Constructor arguments.
   * @return Pointer to the new created widget (never null)
   */
  template<typename T, typename... Args>
  T *AddChild(Args &&...args) {
    static_assert(std::is_base_of_v<Widget, T>, "T must derive from Widget");

    auto child = std::make_unique<T>(std::forward<Args>(args)...);
    auto *ref = child.get();
    AddChild(std::move(child));
    return ref;
  }

  /**
   * @brief Removes a child and returns ownership to the caller.
   *
   * If the supplied widget is not a child of this widget, nullptr is returned.
   *
   * @param child Child widget to remove.
   * @return Owned child widget, or nullptr.
   */
  [[nodiscard]] OwnedWidgetPtrT RemoveChild(Widget *child);

  /**
   * @brief Destroys all children owned by this widget.
   */
  void ClearChildren();

  [[nodiscard]] const WidgetPtrT &Parent() const { return _parent; }
  [[nodiscard]] WidgetPtrT Parent() { return _parent; }

  /**
   * @brief Checks if the specified widget is a child of the current widget.
   *
   * This method determines whether the given widget exists in the current
   * widget's list of children.
   *
   * @param child The widget to check for in the list of children.
   * @return True if the specified widget is a child of the current widget;
   *         false otherwise.
   */
  [[nodiscard]] bool HasChild(const Widget &child) const;

  [[nodiscard]] Vec2D<SIZE_TYPE> Position() const { return _rect.origin; }
  [[nodiscard]] auto Size() const { return _rect.size; }
  [[nodiscard]] RectT<SIZE_TYPE> Rect() const { return _rect; }

  [[nodiscard]] Vec2D<SIZE_TYPE> AbsolutePosition() const {
    return (_parent != nullptr) ? _parent->AbsolutePosition() + Position() : Position();
  }

  [[nodiscard]] RectT<SIZE_TYPE> AbsoluteRect() const {
    return {AbsolutePosition(), Size()};
  }

  [[nodiscard]] RectT<SIZE_TYPE> ComputedRect() const;

  [[nodiscard]] RectT<SIZE_TYPE> AbsoluteComputedRect() const {
    const auto computed = ComputedRect();
    return {AbsolutePosition(), computed.size};
  }

  void Move(const Vec2D<SIZE_TYPE> &position);
  void SetPosition(const Vec2D<SIZE_TYPE> &position) { Move(position); }

  [[nodiscard]] RectT<SIZE_TYPE> GetChildRect() const;

  void Resize(const Vec2D<SIZE_TYPE> &size);
  void SetMinimumSize(const decltype(_min) &newMin);
  [[nodiscard]] auto MinimumSize() const { return _min; }
  void SetMaximumSize(const decltype(_min) &newMax);
  [[nodiscard]] auto MaximumSize() const { return _max; }
  void SetFixedSize(const decltype(_min) &size);

  void SetFocus(bool hasFocus) { /* TODO */ }
  [[nodiscard]] bool HasFocus() const { return _has_focus; }
  [[nodiscard]] bool CanHaveFocus() const { return _can_have_focus; }

  void SetCanProcessActions(bool canProcessActions);
  [[nodiscard]] bool CanProcessActions() const { return _can_process_actions; }
  virtual ActionProcessResult ProcessAction(const ActionInstance &);

  virtual void Paint(Painter &painterObj);
};
}// namespace e00
