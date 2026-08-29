PROJECT(SimpleEngine0GameTest
        VERSION 1.0
        DESCRIPTION "A Simple Game to test the engine"
)


add_executable(SimpleGameTest
    src/main.cpp
        src/ExampleGame.cpp
        src/ExampleGame.hpp
)

target_link_libraries(SimpleGameTest PUBLIC Engine00)
set_target_properties(SimpleGameTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

if (WIN31)
    # Allow the example to call platform::SetMenu (declared in engine private headers).
    target_include_directories(SimpleGameTest PRIVATE ${CMAKE_SOURCE_DIR}/engine/src)
    target_compile_definitions(SimpleGameTest PRIVATE WIN31=1)
endif ()
