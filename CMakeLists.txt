cmake_minimum_required(VERSION 3.15)

project(Engine00
        VERSION 1.0.0
        DESCRIPTION "Small Engine"
        LANGUAGES CXX C)

# Enable position-independent code globally. This is needed because
# some library targets are OBJECT libraries.
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# We are targeting C++17
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


#######################################################################################################################
## CMake Module configuration
#######################################################################################################################
include(ExternalProject)
include(FetchContent)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        ${PROJECT_SOURCE_DIR}/CMakeModules)

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
## Main Engine Library
#######################################################################################################################
add_library(Engine00 STATIC
        include/Engine.hpp
        include/Engine/Config.hpp
        include/Engine/Action.hpp
        include/Engine/Actor.hpp
        include/Engine/Binding.hpp
        include/Engine/ComponentRegistry.hpp
        include/Engine/Engine.hpp
        include/Engine/ActionCategory.hpp
        include/Engine/ActionInstance.hpp
        include/Engine/InputSystem.hpp
        include/Engine/InputEvent.hpp
        include/Engine/Math/SpacePartition.hpp
        include/Engine/Stream.hpp
        include/Engine/World.hpp
        include/Engine/Resource.hpp
        include/Engine/ResourcePtr.hpp
        include/Engine/GameClock.hpp

        include/Engine/Math/Vec2D.hpp
        include/Engine/Math/Rect.hpp
        include/Engine/Math/Color.hpp
        include/Engine/Math/AABB.hpp

        include/Engine/Detail/Property.hpp
        include/Engine/Detail/Array.hpp
        include/Engine/Detail/CircularBuffer.hpp
        include/Engine/Detail/StringFormat.hpp
        include/Engine/Detail/TypeId.hpp

        include/Engine/Resource/Map.hpp
        include/Engine/Resource/Tileset.hpp
        include/Engine/Resource/Bitmap.hpp
        include/Engine/Resource/Font.hpp
        include/Engine/Resource/Sprite.hpp

        include/Engine/GUI/FontGlyph.hpp
        include/Engine/GUI/Menu.hpp
        include/Engine/GUI/Widget.hpp

        include/Engine/Logging/SourceLocation.hpp
        include/Engine/Logging/Logger.hpp

        include/Engine/Scripting/BoxedCast.hpp
        include/Engine/Scripting/BoxedValue.hpp
        include/Engine/Scripting/FunctionParams.hpp
        include/Engine/Scripting/ProxyFunction.hpp
        include/Engine/Scripting/ScriptEngine.hpp
        include/Engine/Scripting/TypeInfo.hpp
        include/Engine/Scripting/detail/AttributeAccess.hpp
        include/Engine/Scripting/detail/CallUtils.hpp
        include/Engine/Scripting/detail/CastHelpers.hpp
        include/Engine/Scripting/detail/FunctionSignature.hpp
        include/Engine/Scripting/detail/HandleReturn.hpp
        include/Engine/Scripting/detail/NativeFunctionT.hpp
        include/Engine/Scripting/detail/Utils.hpp

        src/Engine.cpp
        src/EngineError.cpp
        src/EngineError.hpp
        src/InputEvent.cpp
        src/InternalActions.cpp
        src/InternalActions.hpp
        src/Logger.cpp
        src/Map.cpp
        src/Tileset.cpp
        src/World.cpp
        src/LoadedWorld.cpp
        src/LoadedWorld.hpp
        src/Bitmap_Generic.cpp
        src/IniParser.cpp
        src/IniParser.hpp

        src/GUI/Widget.cpp
        src/GUI/Font.cpp

        src/ScriptEngine/ScriptEngine.cpp
        src/ScriptEngine/Lua/Error.cpp
        src/ScriptEngine/Lua/Lua.hpp
        src/ScriptEngine/Lua/BoxedToLuaConverter.cpp
        src/ScriptEngine/Lua/RefFunction.hpp
        src/ScriptEngine/Lua/LuaProxyFunction.hpp
        src/ScriptEngine/Lua/LuaProxyFunction.cpp
        src/ScriptEngine/Lua/Error.hpp
        src/ScriptEngine/Lua/LuaToBoxedConverter.hpp
        src/ScriptEngine/Lua/LuaScriptEngine.hpp
        src/ScriptEngine/Lua/LuaScriptEngine.cpp
        src/ScriptEngine/Lua/RefFunction.cpp
        src/ScriptEngine/Lua/LuaToNativeTrampoline.hpp
        src/ScriptEngine/Lua/LuaToNativeTrampoline.cpp
        src/ScriptEngine/Lua/NamedFunction.cpp
        src/ScriptEngine/Lua/UserDataHolder.hpp
        src/ScriptEngine/Lua/BoxedToLuaConverter.hpp
        src/ScriptEngine/Lua/TrampolineData.hpp
        src/ScriptEngine/Lua/LuaToBoxedConverter.cpp
        src/ScriptEngine/Lua/UserDataHolder.cpp
        src/ScriptEngine/Lua/NamedFunction.hpp

        src/Loaders/WorldLoader.cpp
        src/Loaders/WorldLoader.hpp
        src/Loaders/BitmapLoader.cpp
        src/Loaders/BitmapLoader.hpp

        src/Resource/ResourceLoader.cpp
        src/Resource/ResourceLoader.hpp
)

set_target_properties(Engine00
        PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS off
)

target_include_directories(Engine00
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
        PRIVATE src/
)

target_compile_definitions(Engine00
        PUBLIC E00_VERSION="${CMAKE_PROJECT_VERSION}"
)

# Add third-party libraries
#target_sources(Engine00 PRIVATE $<TARGET_OBJECTS:>)
target_link_libraries(Engine00 PRIVATE Lua ini nlohmann_json lodepng)
target_link_libraries(Engine00 PUBLIC magic_enum)

#######################################################################################################################
## Add platform code
#######################################################################################################################
add_subdirectory(platforms)


#######################################################################################################################
## Compiler specific options
#######################################################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    message(STATUS "Using GCC")

    target_compile_definitions(Engine00 PRIVATE -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)

    target_compile_options(Engine00 PRIVATE -Wall -pedantic)
    target_compile_options(Engine00 PUBLIC -fasynchronous-unwind-tables)
    target_compile_options(Engine00 PUBLIC -Wl,-pie -Wl,-z,now -Wl,-z,relro)

elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
    message(STATUS "Using CLANG")

    target_compile_definitions(Engine00 PRIVATE -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)
    target_compile_options(Engine00 PUBLIC -Wall -Wno-implicit-fallthrough -Wmissing-prototypes -Wno-unused-command-line-argument -pedantic)
    target_compile_options(Engine00 PUBLIC -fasynchronous-unwind-tables)

elseif (MSVC)
    message(STATUS "Using MSVC")

    option(USE_SPECTRE_MITIGATION "Use specter mitigation" ON)

    target_compile_definitions(Engine00 PUBLIC _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    target_compile_definitions(Engine00 PUBLIC WIN32_LEAN_AND_MEAN)
    target_compile_definitions(Engine00 PUBLIC NOMINMAX)
    target_compile_definitions(Engine00 PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(Engine00 PUBLIC _WIN32_WINNT=0x0602)

    # Force unicode, remove multibyte
    target_compile_definitions(Engine00 PUBLIC UNICODE _UNICODE)
    target_compile_options(Engine00 PUBLIC /UMBCS /U_MBCS)

    # Force C++17
    target_compile_options(Engine00 PUBLIC /std:c++17)

    # Set default calling convention: __cdecl
    target_compile_options(Engine00 PUBLIC /Gd)

    # Don't have minimal re-builds:
    target_compile_options(Engine00 PUBLIC /Gm-)

    # Enable security checks
    target_compile_options(Engine00 PUBLIC /GS)

    # Force standard C++ compliance in for loop scope
    target_compile_options(Engine00 PUBLIC /Zc:forScope)

    # Remove unused code and data
    target_compile_options(Engine00 PUBLIC /Zc:inline)

    # Treat wchar_t as a built-in type
    target_compile_options(Engine00 PUBLIC /Zc:wchar_t)

    # Set default floating point model
    target_compile_options(Engine00 PUBLIC /fp:precise)

    # Allow big object files
    target_compile_options(Engine00 PUBLIC /bigobj)

    # Build with Multiple Processes
    target_compile_options(Engine00 PRIVATE /MP)

    # VS 2017 and higher supports STL-only warning suppressions.
    # A bug in CMake < 3.13.0 may cause the space in this value to
    # cause issues when building with NASM. In that case, update CMake.
    target_compile_definitions(Engine00 PUBLIC "_STL_EXTRA_DISABLED_WARNINGS=4774 4987")

    set(MSVC_DISABLED_WARNINGS_LIST
            "C5045" # mitigation
            "C4577"
            "C4065"
            "C4255"
    )

    # Tell MSVC what to do with third-party includes
    # (We want to silence all warning with)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "/external:I ")
    target_compile_options(Engine00 PUBLIC /external:W0)

    # We need to set a special flag before MSVC 16.10
    # This check is a bit loose; 1930 is VS17 so we might be telling a
    # few versions of 16 to include experimental features, but oh well
    if (MSVC_VERSION LESS 1930)
        target_compile_options(Engine00 PUBLIC /experimental:external)
    endif ()

    if (USE_SPECTRE_MITIGATION)
        target_compile_options(Engine00 PUBLIC /Qspectre)
    endif ()
endif ()

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

