## Figure out life...

if (DJGPP)
    message(STATUS "Building for MS-DOS Protected mode")
    add_subdirectory(dos)
else()
    message(STATUS "Building for the SDL platform")
    add_subdirectory(sdl)
endif ()
