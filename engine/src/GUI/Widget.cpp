#include <Engine.hpp>
#include <utility>

namespace e00 {
Logger Widget::_widget_logger = Logger();

Widget::Widget()
  : _parent(nullptr),
    _rect(RectT<SIZE_TYPE>::maxArea()),
    _min(Vec2D<SIZE_TYPE>::min()),
    _max(Vec2D<SIZE_TYPE>::max()) {
}

Widget::~Widget() {
  _children.clear();
}

bool Widget::HasChild(const Widget &child) const {
  return std::any_of(_children.begin(), _children.end(), [&child](const auto &c) {
    return c == &child;
  });
}


void Widget::SetParent(WidgetPtrT parent) {
  if (_parent) {
    _parent->RemoveChild(this);
  }

  if (parent) {
    parent->InsertChild(this);
  }
}

void Widget::InsertChild(const decltype(_children)::value_type &child) {
  if (child->Parent() == this) {
    _widget_logger.Warning(source_location::current(), "Widget {} already has this {} parent", child->Name(), Name());
    return;
  }

  if (child->Parent()) {
    _widget_logger.Warning(source_location::current(), "Widget {} already has a parent {}", child->Name(), child->Parent()->Name());
    child->Parent()->RemoveChild(child);
  }

  child->_parent = this;
  _children.insert(child);
}

void Widget::RemoveChild(const decltype(_children)::value_type &child) {
  // Remove it from the children list
  for (auto it = _children.begin(); it != _children.end(); ++it) {
    if (*it == child) {
      _children.erase(it);
      break;
    }
  }

  // Reset
  child->_parent = nullptr;
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

void Widget::SetMinimumSize(const decltype(_min) &size) {
  // Is there a minimum set? only continue if the minimum size changed
  if (_has_min_set && size == _min) {
    return;
  }

  // If there is a max set; make sure the minimum is smaller than the max
  if (_has_max_set && size < _max) {
    _max = size;
  }

  // Set
  _has_min_set = true;
  _min = size;

  // make sure the minimum is not bigger than the parent's minimum
  if (_parent && _min > _parent->_max) {
    _min = _parent->_max;
  }

  // Do we still fit?
  if (_rect.size < _min) {
    Resize(_min);
  }
}

void Widget::SetMaximumSize(const decltype(_min) &size) {
  // Do we have a maximum set? only continue if we actually change stuff
  if (_has_max_set && size == _max) {
    return;
  }

  // If we have a minimum; make sure the maximum is bigger than the min
  if (_has_min_set && size < _min) {
    _min = size;
  }

  // Set
  _has_max_set = true;
  _max = size;

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

RectT<Widget::SIZE_TYPE> Widget::GetChildRect() const {
  RectT<SIZE_TYPE> rect = _rect;

  for (const auto &child : _children) {
    rect = rect.Unite(child->_rect);
  }

  return rect;
}


void Widget::SetCanProcessActions(bool canProcessActions) {
  this->_can_process_actions = canProcessActions;

  if (canProcessActions && _parent && !_parent->CanProcessActions()) {
    _parent->SetCanProcessActions(true);
  }
}

Widget::ActionProcessResult Widget::ProcessAction(const ActionInstance &action) {
  if (!_can_process_actions) {
    return ActionProcessResult::NotHandled;
  }

  auto ret = ActionProcessResult::NotHandled;

  for (const auto &child : _children) {
    if (!child->CanProcessActions())
      continue;

    switch (child->ProcessAction(action)) {
      case ActionProcessResult::NotHandled: break;
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
  if (_background_type == BackgroundType::Solid) {
    painterObj.setPen(Painter::PenStyle::NoPen);
    painterObj.setBrushColor(_background_color);
    painterObj.fillRect(_rect);
  } else if (_background_type == BackgroundType::Image) {
  }
}

}// namespace e00
