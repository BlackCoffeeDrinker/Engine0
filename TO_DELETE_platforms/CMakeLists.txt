## Figure out life...

add_library(Platform STATIC)
target_link_libraries(Platform PUBLIC Engine00Interface)
target_include_directories(Platform PUBLIC backend_includes)

if (DJGPP)
    message(STATUS "Building for MS-DOS Protected mode")
    add_subdirectory(dos)
elseif (APPLE)
    message(STATUS "Building for Apple OSX")
    add_subdirectory(osx)
else ()
    message(STATUS "Building for the SDL platform")
    add_subdirectory(sdl)
endif ()

add_library(Engine00WithPlatform INTERFACE)
target_link_libraries(Engine00WithPlatform INTERFACE Platform Engine00)
