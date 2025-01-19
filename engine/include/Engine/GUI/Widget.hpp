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
  using WidgetPtrT = Widget *;

public:
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

  static Logger _widget_logger;

  std::string _name;
  WidgetPtrT _parent;
  std::set<WidgetPtrT> _children;

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
  void InsertChild(const decltype(_children)::value_type &child);
  void RemoveChild(const decltype(_children)::value_type &child);

  virtual void ResizeEvent() {
    _has_computed_size = false;
    for (auto &child : _children) {
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
  explicit Widget();
  virtual ~Widget();

  void SetName(std::string &&name) { _name = std::move(name); }
  [[nodiscard]] const auto &Name() const { return _name; }

  /**
   * @brief Sets the parent of the current widget.
   *
   * This method assigns a parent widget to the current widget, establishing
   * a parent-child relationship.
   *
   * If the widget already has a parent, it will first remove itself from
   * the current parent's children list before being inserted as a child in
   * the new parent.
   *
   * @param parent A pointer to the widget to be set as the parent. Passing
   *        a nullptr will detach the current widget from its parent.
   */
  void SetParent(WidgetPtrT parent);
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

  [[nodiscard]] auto Position() const { return _rect.origin; }
  [[nodiscard]] auto Size() const { return _rect.size; }
  [[nodiscard]] auto AbsolutePosition() const {
    return (_parent != nullptr) ? _parent->Position() + Position() : Position();
  }

  [[nodiscard]] RectT<SIZE_TYPE> GetChildRect() const;

  void Resize(const Vec2D<SIZE_TYPE> &size);
  void SetMinimumSize(const decltype(_min) &size);
  [[nodiscard]] auto MinimumSize() const { return _min; }
  void SetMaximumSize(const decltype(_min) &size);
  [[nodiscard]] auto MaximumSize() const { return _max; }
  void SetFixedSize(const decltype(_min) &size);

  void SetFocus(bool hasFocus) { /* TODO */ }
  [[nodiscard]] bool HasFocus() const { return _has_focus; }
  [[nodiscard]] bool CanHaveFocus() const { return _can_have_focus; }

  void SetCanProcessActions(bool canProcessActions);
  [[nodiscard]] bool CanProcessActions() const { return _can_process_actions; }
  ActionProcessResult ProcessAction(const ActionInstance &);

  virtual void Paint(Painter &painterObj);
};
}// namespace e00
