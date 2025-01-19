#pragma once

namespace e00 {
/*
*  struct ActionBuffer {
    constexpr static auto MAX_SIZE = 254;

    void Clear() {
      processed = at = 0;
    }

    template<typename Func>
    void ForEach(Func func) {
      for (decltype(actions)::size_type i = processed; i < at; i++) {
        func(actions.at(i));
      }
    }

    void Insert(const ActionInstance &a) {
      actions.at(at) = a;
      at = (at + 1) % actions.size();
    }

    [[nodiscard]] bool Empty() const {
      return at <= processed;
    }

    ActionInstance Pop() {
      if (Empty()) {
        return {};
      }

      return actions.at(processed++);
    }

  private:
    std::array<ActionInstance, MAX_SIZE> actions;
    std::array<ActionInstance, MAX_SIZE>::size_type at = 0;
    std::array<ActionInstance, MAX_SIZE>::size_type processed = 0;
  };
*/

/**
 * ActionInstance is an action that was triggered at a point in time
 */
struct ActionInstance {
  Action action;

  GameClock::time_point when;

  constexpr ActionInstance() = default;

  constexpr ActionInstance(Action action, GameClock::time_point when) : action(action), when(when) {}

  bool operator<(const ActionInstance &rhs) const noexcept { return when < rhs.when; }
};
}// namespace e00
