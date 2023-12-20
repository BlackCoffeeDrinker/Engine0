#include <catch2/catch.hpp>
#include <Engine.hpp>

TEST_CASE("Components Can Be Added To ControlBlock") {
  e00::ComponentRegistry container;
  class TestComponent {};
  auto* c = container.CreateComponent<TestComponent>();
  REQUIRE(c != nullptr);
}

TEST_CASE("Components Can Be Retrieved From ControlBlock") {
  e00::ComponentRegistry container;
  class TestComponent {};
  auto* c = container.CreateComponent<TestComponent>();
  REQUIRE(c != nullptr);
  REQUIRE(container.GetComponent<TestComponent>() == c);
}

TEST_CASE("Components Are Deleted With ControlBlock") {
  class TestComponent  {
    int &c;

  public:
    explicit TestComponent(int &c) : c(c) { c++; }
    ~TestComponent() { c--; }
  };

  int c = 0;

  {
    e00::ComponentRegistry container;
    container.CreateComponent<TestComponent>(c);
    REQUIRE(c == 1);
  }

  REQUIRE(c == 0);
}

TEST_CASE("Components Types Are Unique Within a ControlBlock") {
  e00::ComponentRegistry container;
  class TestComponent {};

  auto* c = container.CreateComponent<TestComponent>();
  REQUIRE(c != nullptr);

  auto* c2 = container.CreateComponent<TestComponent>();
  REQUIRE(c2 == nullptr);

  REQUIRE(container.GetComponent<TestComponent>() == c);
}

TEST_CASE("Components is NULL if a type is not found") {
  e00::ComponentRegistry container;
  class TestComponent  {};
  REQUIRE(container.GetComponent<TestComponent>() == nullptr);
}

