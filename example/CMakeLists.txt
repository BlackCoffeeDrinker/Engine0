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
