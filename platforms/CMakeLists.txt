## Figure out life...

if (DJGPP)
    message(STATUS "Building for MS-DOS Protected mode")
else()
    message(STATUS "Building for the SDL platform")
    add_subdirectory(../vendor/SDL ${CMAKE_BINARY_DIR}/SDLBuild EXCLUDE_FROM_ALL)
    add_subdirectory(sdl)
endif ()
