#include <catch2/catch.hpp>

#include <iostream>
#include <Engine.hpp>

namespace {
bool returns_true() {
  return true;
}

bool accepts_an_int(int i) {
  return i == 3;
}
}// namespace

TEST_CASE("Type Info identifies pointers") {
  int a = 0;
  int *p = &a;

  auto info = e00::scripting::user_type(p);
  REQUIRE(info.is_pointer());
}

TEST_CASE("Script System initializes") {
  auto script = e00::ScriptEngine::Create();
  REQUIRE(script);
}

TEST_CASE("Can log from script") {
  auto script = e00::ScriptEngine::Create();
  script->parse("\nprint(\"Hello World\")\n");
}

TEST_CASE("Can call free functions") {
  auto script = e00::ScriptEngine::Create();

  script->register_function("returns_true", &returns_true);
  script->parse("\nreturns_true()\n");
}

TEST_CASE("Passing an int to a function") {
  auto script = e00::ScriptEngine::Create();

  script->register_function("accepts_an_int", &accepts_an_int);
  script->parse("\naccepts_an_int(3)\n");
}

TEST_CASE("Calling an int lamda") {
  auto script = e00::ScriptEngine::Create();
  script->register_function("l", [](int a) {
    return a == 4;
  });
  script->parse("\nl(4)\n");
}

TEST_CASE("Calling a const string lamda") {
  auto script = e00::ScriptEngine::Create();
  script->register_function("l", [](const std::string &a) {
    return a.size();
  });
  script->parse("\nl(\"A string\")\n");
}

TEST_CASE("Gets native return value correctly") {
  auto script = e00::ScriptEngine::Create();
  script->register_function("a", []() { return 3; });
  script->register_function("b", [](int a) {
    return a == 3;
  });

  script->parse("\nb(a())\n");
}

TEST_CASE("Multiple types of native methods can be called") {
  auto script = e00::ScriptEngine::Create();
  int a = 0;
  bool is_failed = false;
  script->register_function("a", [&a]() { return a; });
  script->register_function("b", [&a](int aa) { a = aa; });
  script->register_function("VALIDATE", [&is_failed](bool a) { is_failed = !a; });

  script->parse("\nb(4)\nVALIDATE(a() == 4)\n");
  REQUIRE_FALSE(is_failed);
}

TEST_CASE("Native calls a method in script and gets it's return value") {
  auto script = e00::ScriptEngine::Create();
  script->parse("\nfunction test ()\n return \"Hello, World\"\nend\n");

  auto hello_world = script->call<std::string>("test");
  REQUIRE(hello_world == "Hello, World");
}

TEST_CASE("Register a variable") {
  int a = 5;
  int got_a = 0;

  auto script = e00::ScriptEngine::Create();
  script->register_variable("a", a);
  script->register_function("ensure_same", [&](int i) { got_a = i;});

  REQUIRE_FALSE(a == got_a);
  script->parse("\nensure_same(a)\n");
  REQUIRE(got_a == a);
}

TEST_CASE("Pass a script function back to native and call it") {
  auto script = e00::ScriptEngine::Create();

  e00::scripting::BoxedValue val;

  script->register_function("test", [&val](e00::scripting::ProxyFunction *proxy_function) {
    val = proxy_function->operator()(4);
  });
  script->parse("\nhalf = function(x)\nreturn x / 2\nend\n\ntest(half)\n");

  REQUIRE(val.is_arithmetic());
  if (val.is_arithmetic()) {
    int result = 0;
    e00::scripting::try_cast<int>(val, [&result](int a) {
      result = a;
    });

    REQUIRE(result == 2);
  }
}

TEST_CASE("Native can return structs") {
  auto script = e00::ScriptEngine::Create();

  struct Item {
    int id;
    std::string name;
  };

  bool is_ok = false;

  script->register_function("make_item", []() {
    return Item{ 1, "Fred" };
  });

  script->register_function("print_item_name", [&is_ok](const Item &item) {
    is_ok = (item.id == 1);
  });

  REQUIRE_FALSE(is_ok);
  script->parse("\nprint_item_name(make_item())\n");
  REQUIRE(is_ok);
}

TEST_CASE("Can call object methods") {
  class Simple {
  public:
    bool called = false;
    void call() { called = true; }
  };


  auto script = e00::ScriptEngine::Create();
  script->register_function("doit", &Simple::call);

  auto r = new Simple;
  REQUIRE_FALSE(r->called);

  script->register_variable("a", r);
  script->parse("\na:doit()\n");

  REQUIRE(r->called);
  delete r;
}

/*
TEST_CASE("Can access structs value") {
  struct test {
    int a;
    int b;
  };
  test t{};
  t.a = 4;
  t.b = 3;

  bool is_failed = false;

  auto script = e00::impl::ScriptEngine::Create();
  script->register_function("VALIDATE", [&is_failed](bool a) { is_failed = !a; });
  script->register_type<test>();
  script->register_function("a", &test::a);
  script->register_function("a", &test::b);
  script->register_variable("test_a", &t);

  script->parse("\nVALIDATE(test_a.a == 4)\n");
  if (is_failed) {
    FAIL();
  }
}
*/
