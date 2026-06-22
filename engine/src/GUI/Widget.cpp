#include <Engine.hpp>
#include <utility>

namespace {
template<typename T>
constexpr T clampValue(T value, T minValue, T maxValue) {
  return std::min(std::max(value, minValue), maxValue);
}
}// namespace


namespace e00 {
Widget::Widget()
    : _parent(nullptr),
      _rect(RectT<SIZE_TYPE>::maxArea()),
      _min(Vec2D<SIZE_TYPE>::min()),
      _max(Vec2D<SIZE_TYPE>::max()) {
}

Widget::Widget(std::string name)
    : _name(std::move(name)),
      _parent(nullptr),
      _rect(RectT<SIZE_TYPE>::maxArea()),
      _min(Vec2D<SIZE_TYPE>::min()),
      _max(Vec2D<SIZE_TYPE>::max()) {
}

Widget::~Widget() {
  ClearChildren();

  if (_parent) {
    auto detached = _parent->RemoveChild(this);
    detached.release();
  }
}

bool Widget::HasChild(const Widget &child) const {
  return std::ranges::any_of(_children, [&child](const auto &c) {
    return c.get() == &child;
  });
}

Widget *Widget::AddChild(OwnedWidgetPtrT child) {
  if (!child) {
    throw std::invalid_argument("Cannot add a null child widget");
  }

  if (child.get() == this) {
    throw std::invalid_argument("Cannot add a widget as its own child");
  }

  if (child->Parent() == this) {
    GetDefaultLogger().Warning(source_location::current(), "Widget {} already has this {} parent", child->Name(), Name());
    return child.get();
  }

  if (child->Parent()) {
    GetDefaultLogger().Warning(source_location::current(), "Widget {} already has a parent {}", child->Name(), child->Parent()->Name());
    child = child->Parent()->RemoveChild(child.get());
  }

  child->_parent = this;
  auto ref = child.get();
  _children.emplace_back(std::move(child));
  ResizeEvent();

  return ref;
}

Widget::OwnedWidgetPtrT Widget::RemoveChild(Widget *child) {
  const auto it = std::ranges::find_if(_children, [&child](const auto &candidate) {
    return candidate.get() == child;
  });

  if (it == _children.end()) {
    return nullptr;
  }

  auto removed = std::move(*it);
  _children.erase(it);
  removed->_parent = nullptr;
  ResizeEvent();

  return removed;
}

void Widget::ClearChildren() {
  for (auto &child: _children) {
    child->_parent = nullptr;
  }

  _children.clear();
}

RectT<Widget::SIZE_TYPE> Widget::GetChildRect() const {
  RectT<SIZE_TYPE> rect = _rect;

  for (const auto &child: _children) {
    rect = rect.Unite(child->_rect);
  }

  return rect;
}

RectT<Widget::SIZE_TYPE> Widget::ComputedRect() const {
  RectT<SIZE_TYPE> computed = _rect;

  Vec2D<SIZE_TYPE> minSize = _has_min_set ? _min : Vec2D<SIZE_TYPE>::min();
  Vec2D<SIZE_TYPE> maxSize = _has_max_set ? _max : Vec2D<SIZE_TYPE>::max();

  if (_parent) {
    const auto parentRect = _parent->ComputedRect();

    const auto availableWidth =
        computed.origin.x >= parentRect.size.x
            ? SIZE_TYPE{0}
            : static_cast<SIZE_TYPE>(parentRect.size.x - computed.origin.x);

    const auto availableHeight =
        computed.origin.y >= parentRect.size.y
            ? SIZE_TYPE{0}
            : static_cast<SIZE_TYPE>(parentRect.size.y - computed.origin.y);

    maxSize.x = std::min(maxSize.x, availableWidth);
    maxSize.y = std::min(maxSize.y, availableHeight);
  }

  computed.size.x = clampValue(computed.size.x, minSize.x, maxSize.x);
  computed.size.y = clampValue(computed.size.y, minSize.y, maxSize.y);

  return computed;
}


void Widget::Move(const Vec2D<SIZE_TYPE> &position) {
  if (_rect.origin == position) {
    return;
  }

  _rect.origin = position;
  ResizeEvent();
}

void Widget::Resize(const Vec2D<SIZE_TYPE> &size) {
  Vec2D<SIZE_TYPE> new_size = size;

  if (_has_min_set) {
    new_size.x = std::max(new_size.x, _min.x);
    new_size.y = std::max(new_size.y, _min.y);
  }

  if (_has_max_set) {
    new_size.x = std::min(new_size.x, _max.x);
    new_size.y = std::min(new_size.y, _max.y);
  }

  // Check if the size has actually changed
  if (new_size != _rect.size) {
    _rect.size = new_size;

    // Inform all children about this event
    ResizeEvent();
  }
}

void Widget::SetMinimumSize(const decltype(_min) &newMin) {
  // Is there a minimum set? only continue if the minimum size changed
  if (_has_min_set && newMin == _min) {
    return;
  }

  // If there is a max set; make sure the minimum is smaller than the max
  /*
  if (_has_max_set && _max < newMin) {
    _max = newMin;
  }
  */

  // Set
  _has_min_set = true;
  _min = newMin;

  // make sure the minimum is not bigger than the parent's minimum
  if (_parent && _min > _parent->_max) {
    _min = _parent->_max;
  }

  // Do we still fit?
  if (_rect.size < _min) {
    Resize(_min);
  }
}

void Widget::SetMaximumSize(const decltype(_max) &newMax) {
  // Do we have a maximum set? only continue if we actually change stuff
  if (_has_max_set && newMax == _max) {
    return;
  }

  // If we have a minimum; make sure the maximum is bigger than the min
  /*
  if (_has_min_set && _min > newMax) {
    _min = newMax;
  }
  */

  // Set
  _has_max_set = true;
  _max = newMax;

  // Make sure we're not setting bigger than parent
  if (_parent && _max > _parent->_max) {
    _max = _parent->_max;
  }

  // Does the widget still fit?
  if (_rect.size > _max) {
    Resize(_max);
  }
}

void Widget::SetFixedSize(const decltype(_min) &size) {
  _has_min_set = _has_max_set = true;
  _min = size;
  _max = size;
  _computed_size = size;
  Resize(size);
}

void Widget::SetCanProcessActions(bool canProcessActions) {
  this->_can_process_actions = canProcessActions;

  if (canProcessActions) {
    auto p = _parent;
    while (p) {
      p->_can_process_actions = true;
      p = p->_parent;
    }
  }
}

Widget::ActionProcessResult Widget::ProcessAction(const ActionInstance &action) {
  if (!_can_process_actions) {
    return ActionProcessResult::NotHandled;
  }

  auto ret = ActionProcessResult::NotHandled;

  for (const auto &child: _children) {
    if (!child->CanProcessActions())
      continue;

    switch (child->ProcessAction(action)) {
      case ActionProcessResult::NotHandled:
        break;
      case ActionProcessResult::Handled:
        ret = ActionProcessResult::Handled;
        break;
      case ActionProcessResult::HandledAndConsumed:
        return ActionProcessResult::HandledAndConsumed;
    }
  }

  return ret;
}

void Widget::Paint(Painter &painterObj) {
  const auto paintRect = AbsoluteComputedRect();

  if (_background_type == BackgroundType::Solid) {
    painterObj.SetNoPen();
    painterObj.SetBrushColor(_background_color);
    painterObj.DrawRect(paintRect);
  } else if (_background_type == BackgroundType::Image) {
  }

  for (auto &child: _children) {
    child->Paint(painterObj);
  }
}

}// namespace e00
