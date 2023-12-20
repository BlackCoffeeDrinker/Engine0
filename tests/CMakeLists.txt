include(FetchContent)

# Testing library
FetchContent_Declare(
        catch
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.6)

FetchContent_MakeAvailable(catch)

list(APPEND CMAKE_MODULE_PATH ${catch_SOURCE_DIR}/contrib)
# Adds Catch2::Catch2


add_executable(Engine00_Tests
        tests.cpp
        catch_main.cpp
        test_components.cpp
        test_action.cpp
        test_engine.cpp
        test_map.cpp
        tests_script.cpp test_format.cpp test_sourcelocation.cpp test_data.cpp test_world.cpp)
target_include_directories(Engine00_Tests PRIVATE ../src)
target_link_libraries(Engine00_Tests
        PRIVATE Engine00
        PRIVATE Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(Engine00_Tests)
