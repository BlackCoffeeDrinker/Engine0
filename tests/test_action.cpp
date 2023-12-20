#include <catch2/catch.hpp>
#include <Engine.hpp>
#include "magic_enum.hpp"

enum class TestActions : int {
  TEST1 = 1,
  TEST2,
  TEST3
};

namespace e00 {
template<>
struct IsActionTypeEnum<TestActions> : std::true_type {};
}

class ACategory : public e00::ActionCategory {
public:
  [[nodiscard]] std::string_view name() const noexcept override {
    return "TestActionCategory";
  }

  [[nodiscard]] std::string_view message(uint32_t binding) const override {
    return magic_enum::enum_name(static_cast<TestActions>(binding));
  }
};

ACategory a_category{};

e00::Action make_action(TestActions e) {
  return {e, a_category};
}

TEST_CASE("Actions are equal if they represent the same enum") {
  auto action1 = make_action(TestActions::TEST1);
  auto action1_1 = make_action(TestActions::TEST1);
  REQUIRE(action1 == action1_1);
}

TEST_CASE("Actions are unique") {
  auto action1 = make_action(TestActions::TEST1);

  auto action2 = make_action(TestActions::TEST2);
  REQUIRE(action1 != action2);

  auto action3 = make_action(TestActions::TEST3);
  REQUIRE(action1 != action3);
}

TEST_CASE("Actions are bindable") {
  auto action1 = make_action(TestActions::TEST1);
  int c = 0;
  auto binding1 = e00::make_binding(action1, [&c]() { c++; });
  (*binding1)();
  REQUIRE(c == 1);
}
