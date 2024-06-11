cmake_minimum_required(VERSION 3.15)

project(Engine00
        VERSION 1.0.0
        DESCRIPTION "Small Engine"
        LANGUAGES CXX C)

#######################################################################################################################
## Global Project targeting
#######################################################################################################################
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
# Enable position-independent code globally. This is needed because
# some library targets are OBJECT libraries.
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

#######################################################################################################################
## CMake Module configuration
#######################################################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalProject)
include(FetchContent)
include(cmake/CPM.cmake)

#######################################################################################################################
## Useful stuff to set only if this is the main project and not included through add_subdirectory
#######################################################################################################################
IF (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Make sure cmake uses -std=c++ and not -std=g++
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    include(CTest)

    FIND_PACKAGE(Doxygen)
    if (Doxygen_FOUND)
        add_subdirectory(docs)
    else ()
        message(STATUS "Doxygen not found, not building docs")
    endif ()
endif ()

#######################################################################################################################
## Load up vendors
#######################################################################################################################
add_subdirectory(vendor)

#######################################################################################################################
## Main Engine
#######################################################################################################################
add_subdirectory(engine)

#######################################################################################################################
## Tests
#######################################################################################################################
# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
        AND BUILD_TESTING)
    add_subdirectory(tests)
endif ()

#######################################################################################################################
## Example project
#######################################################################################################################
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
        AND BUILD_TESTING)
    message("Building example project")
    add_subdirectory(example)
endif ()

#######################################################################################################################
## Make INSTALL/EXPORT targets
#######################################################################################################################
#[[

TODO

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${PROJECT_BINARY_DIR}/EngineConfigVersion.cmake"
	VERSION ${CMAKE_PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)

install(TARGETS Engine00
	EXPORT EngineExports
	LIBRARY DESTINATION lib COMPONENT Runtime
	ARCHIVE DESTINATION lib COMPONENT Development
	RUNTIME DESTINATION bin COMPONENT Runtime
	PUBLIC_HEADER DESTINATION include COMPONENT Development
	BUNDLE DESTINATION bin COMPONENT Runtime
)

configure_package_config_file(
	"${PROJECT_SOURCE_DIR}/cmake/Engine00Config.cmake.in"
	"${PROJECT_BINARY_DIR}/Engine00Config.cmake"
	INSTALL_DESTINATION lib/cmake/Engine00
)

install(EXPORT EngineExports DESTINATION lib/cmake/Engine00)

install(FILES "${PROJECT_BINARY_DIR}/Engine00ConfigVersion.cmake"
	"${PROJECT_BINARY_DIR}/Engine00Config.cmake"
	DESTINATION lib/cmake/Engine00)

]]

