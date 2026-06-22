include(FetchContent)

# Testing library
FetchContent_Declare(
        catch
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.0)

FetchContent_MakeAvailable(catch)

list(APPEND CMAKE_MODULE_PATH ${catch_SOURCE_DIR}/contrib)
# Adds Catch2::Catch2

add_executable(Engine00_Tests
        tests.cpp
        test_action.cpp
        test_engine.cpp
        test_map.cpp
        tests_script.cpp 
        test_format.cpp
        test_sourcelocation.cpp 
        test_data.cpp 
        test_world.cpp
        test_widget.cpp
        test_gifloader.cpp
        test_font.cpp
        test_blitting.cpp
        test_painter.cpp
        test_no_palette.cpp
        tests.hpp)
target_include_directories(Engine00_Tests PRIVATE ../engine/src)
target_link_libraries(Engine00_Tests
        PRIVATE Engine00
        PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(Engine00_Tests)
