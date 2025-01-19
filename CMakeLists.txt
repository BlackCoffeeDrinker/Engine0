cmake_minimum_required(VERSION 3.15)

project(Engine00
        VERSION 1.0.0
        DESCRIPTION "Small Engine"
        LANGUAGES CXX C)

include(cmake/PreventInSourceBuilds.cmake)

#######################################################################################################################
## Global Project targeting
#######################################################################################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Enable position-independent code globally. This is needed because
# some library targets are OBJECT libraries.
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

#######################################################################################################################
## CMake Module configuration
#######################################################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        ${PROJECT_SOURCE_DIR}/cmake)

include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(ExternalProject)
include(FetchContent)

include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(cmake/CPM.cmake)

include(ProjectOptions.cmake)

#######################################################################################################################
## Setup Env
#######################################################################################################################
e0_setup_options()
e0_global_options()
e0_local_options()

set(GIT_SHA "Unknown" CACHE STRING "SHA this build was generated from")
string(SUBSTRING "${GIT_SHA}" 0 8 GIT_SHORT_SHA)

target_compile_features(e0_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(e0::e0_options ALIAS e0_options)
add_library(e0::e0_warnings ALIAS e0_warnings)

#######################################################################################################################
## Useful stuff to set only if this is the main project and not included through add_subdirectory
#######################################################################################################################
IF (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
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

