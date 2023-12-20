#pragma once

namespace e00 {
namespace detailsp {
  struct Node;

  struct NodeItem {
    uint32_t item;
    Vec2D<WorldCoordinateType> point;
    NodeItem *next;
    Node *node;

    constexpr NodeItem() : item{}, point{}, next(nullptr), node(nullptr) {}
  };

  struct Node {
    const RectT<WorldCoordinateType> boundaries;
    const Node *parent;

    NodeItem *itemStart;
    std::array<Node *, 4> children;

    Node(Node *parent, const RectT<WorldCoordinateType> &boundaries)
      : boundaries(boundaries),
        parent(parent),
        itemStart(nullptr),
        children{ nullptr, nullptr, nullptr, nullptr } {}

    ~Node() {
      for (auto &i : children)
        delete i;

      while (itemStart) {
        auto *next = itemStart->next;
        delete itemStart;
        itemStart = next;
      }
    }

    template<typename... Args>
    void CreateChildNodes(const Vec2D<WorldCoordinateType> &size, const Args &...args) {
      children = { new Node(this, { args, size })... };

      Rebalance();
    }

    NodeItem *CreateItemHere(const Vec2D<WorldCoordinateType> &pos) {
      auto *item = new NodeItem();
      item->next = itemStart;
      item->node = this;
      item->point = pos;
      return item;
    }

    Node *FindFor(const Vec2D<WorldCoordinateType> &pos) {
      // Not here!
      if (!boundaries.Contains(pos)) return nullptr;

      // Do we have child nodes? If so it must be one of them
      if (children[0]) {
        for (auto child : children)
          if (auto n = child->FindFor(pos)) return n;
        return nullptr;
      }

      // Otherwise, it's us
      return this;
    }

    [[nodiscard]] size_t CountItems() const {
      if (children[0]) {
        size_t total = 0;
        for (const auto i : children)
          total += i->CountItems();
        return total;
      }

      if (!itemStart) return 0;

      size_t num = 0;
      for (auto *item = itemStart; item; item = item->next) {
        num++;
      }

      return num;
    }

    void Rebalance() {
      while (itemStart) {
        auto *next = itemStart->next;

        if (auto n = FindFor(itemStart->point)) {
          if (n != this) {
            itemStart->node = n;
            itemStart->next = n->itemStart;
            n->itemStart = itemStart;
          }
        }

        itemStart = next;
      }
    }
  };

}// namespace detailsp

template<typename T, std::size_t MAX_ITEMS>
class QuadTree {
  detailsp::Node _root;
  std::vector<T> _items;

  void Split(detailsp::Node *n) const {
    // Create children
    auto quadrantSize = n->boundaries.size / static_cast<WorldCoordinateType>(2);

    n->CreateChildNodes(quadrantSize,
      Vec2D<WorldCoordinateType>(n->boundaries.origin.x + quadrantSize.x, n->boundaries.origin.y),
      n->boundaries.origin,
      Vec2D<WorldCoordinateType>(n->boundaries.origin.x, n->boundaries.origin.y + quadrantSize.y),
      Vec2D<WorldCoordinateType>(n->boundaries.origin + quadrantSize));
  }

public:
  explicit QuadTree(const RectT<WorldCoordinateType> &boundaries) : _root(nullptr, boundaries) {
  }

  ~QuadTree() = default;

  bool Insert(T item, const Vec2D<WorldCoordinateType> &pos);
};

template<typename T, std::size_t MAX_ITEMS>
bool QuadTree<T, MAX_ITEMS>::Insert(T item, const Vec2D<e00::WorldCoordinateType> &pos) {
  if (auto n = _root.FindFor(pos)) {
    auto *nodeItem = n->CreateItemHere(pos);
    _items.push_back(item);
    nodeItem->item = _items.size() - 1;

    if (n->CountItems() >= MAX_ITEMS) {
      Split(n);
    }

    return true;
  }

  return false;
}

}// namespace e00
