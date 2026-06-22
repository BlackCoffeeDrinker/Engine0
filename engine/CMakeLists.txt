#######################################################################################################################
## Declare main target
#######################################################################################################################
add_library(Engine00 STATIC
        src/Platform.hpp
        src/Platform.cpp
        src/PrivateInclude.hpp

        src/bootstrap.cpp
        src/Engine.cpp
        src/EngineError.cpp
        src/EngineError.hpp
        src/InputEvent.cpp
        src/InternalActions.cpp
        src/InternalActions.hpp
        src/Map.cpp
        src/World.cpp
        src/Bitmap_Generic.cpp
        src/IniParser.cpp
        src/IniParser.hpp
        src/ResourceManager.cpp
        src/StreamFactory.cpp
        src/DrawableSurface.cpp

        src/GUI/Widget.cpp
        src/GUI/Font.cpp
        src/GUI/WorldWidget.cpp
        src/GUI/LabelWidget.cpp

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
        src/Loaders/SpriteGifLoader.cpp
        src/Loaders/SpriteGifLoader.hpp
        src/Loaders/PngLoader.cpp
        src/Loaders/PngLoader.hpp
        src/Loaders/PaletteLoader.cpp
        src/Loaders/PaletteLoader.h

        src/Resource/ResourceLoader.cpp
        src/Resource/Sprite.cpp

        src/Painter_PaintDevice.hpp
        src/Painter_PaintDevice.cpp
        src/BitmapData.cpp
        src/BitmapData.hpp
        src/Logger.cpp
        src/TranslatableText.cpp
        src/TranslatableText.hpp
        src/Painter_Generic.cpp

)

target_include_directories(Engine00
        PRIVATE src/
)

add_library(Engine00Interface INTERFACE)

set_target_properties(Engine00Interface
        PROPERTIES
        CXX_STANDARD 23
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE True
)

target_include_directories(Engine00Interface
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
)

target_compile_definitions(Engine00Interface
        INTERFACE E00_VERSION="${CMAKE_PROJECT_VERSION}"
)

# Add third-party libraries
target_link_libraries(Engine00Interface INTERFACE magic_enum)
target_link_libraries(Engine00 PRIVATE Lua ini nlohmann_json::nlohmann_json lodepng zlibstatic)
target_link_libraries(Engine00 PUBLIC Engine00Interface)

# Disable RTTI for this target across different compilers
if (MSVC)
    target_compile_options(Engine00 PUBLIC "/GR-")
else ()
    target_compile_options(Engine00 PUBLIC "-fno-rtti")
endif ()

if (DJGPP)
    target_compile_definitions(Engine00 PUBLIC "-DLUA_32BITS")
endif ()

#######################################################################################################################
## Compiler specific options
#######################################################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    message(STATUS "Using GCC")

    target_compile_definitions(Engine00 PRIVATE -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)
    target_compile_options(Engine00 PRIVATE -Wall -pedantic)

    target_compile_options(Engine00Interface INTERFACE -fasynchronous-unwind-tables)
    target_compile_options(Engine00Interface INTERFACE -Wl,-pie -Wl,-z,now -Wl,-z,relro)

elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
    message(STATUS "Using CLANG")

    target_compile_definitions(Engine00 PRIVATE -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)

    target_compile_options(Engine00Interface INTERFACE -Wall -Wno-implicit-fallthrough -Wmissing-prototypes -Wno-unused-command-line-argument -pedantic)
    target_compile_options(Engine00Interface INTERFACE -fasynchronous-unwind-tables)

elseif (MSVC)
    message(STATUS "Using MSVC")

    option(USE_SPECTRE_MITIGATION "Use specter mitigation" ON)

    target_compile_definitions(Engine00Interface INTERFACE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    target_compile_definitions(Engine00Interface INTERFACE WIN32_LEAN_AND_MEAN)
    target_compile_definitions(Engine00Interface INTERFACE NOMINMAX)
    target_compile_definitions(Engine00Interface INTERFACE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(Engine00Interface INTERFACE _WIN32_WINNT=0x0602)

    # Force unicode, remove multibyte
    target_compile_definitions(Engine00Interface INTERFACE UNICODE _UNICODE)
    target_compile_options(Engine00Interface INTERFACE /UMBCS /U_MBCS)

    # Force C++20
    target_compile_options(Engine00Interface INTERFACE /std:c++20)

    # Set default calling convention: __cdecl
    target_compile_options(Engine00Interface INTERFACE /Gd)

    # Don't have minimal re-builds:
    target_compile_options(Engine00Interface INTERFACE /Gm-)

    # Enable security checks
    target_compile_options(Engine00Interface INTERFACE /GS)

    # Force standard C++ compliance in for loop scope
    target_compile_options(Engine00Interface INTERFACE /Zc:forScope)

    # Remove unused code and data
    target_compile_options(Engine00Interface INTERFACE /Zc:inline)

    # Treat wchar_t as a built-in type
    target_compile_options(Engine00Interface INTERFACE /Zc:wchar_t)

    # Set default floating point model
    target_compile_options(Engine00Interface INTERFACE /fp:precise)

    # Allow big object files
    target_compile_options(Engine00Interface INTERFACE /bigobj)

    # VS 2017 and higher supports STL-only warning suppressions.
    # A bug in CMake < 3.13.0 may cause the space in this value to
    # cause issues when building with NASM. In that case, update CMake.
    target_compile_definitions(Engine00Interface INTERFACE "_STL_EXTRA_DISABLED_WARNINGS=4774 4987")

    set(MSVC_DISABLED_WARNINGS_LIST
            "C5045" # mitigation
            "C4577"
            "C4065"
            "C4255"
    )

    # Tell MSVC what to do with third-party includes
    # (We want to silence all warning with)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "/external:I ")
    target_compile_options(Engine00Interface INTERFACE /external:W0)

    # We need to set a special flag before MSVC 16.10
    # This check is a bit loose; 1930 is VS17 so we might be telling a
    # few versions of 16 to include experimental features, but oh well
    if (MSVC_VERSION LESS 1930)
        target_compile_options(Engine00Interface INTERFACE /experimental:external)
    endif ()

    if (USE_SPECTRE_MITIGATION)
        target_compile_options(Engine00Interface INTERFACE /Qspectre)
    endif ()
endif ()

#######################################################################################################################
## Sources
#######################################################################################################################
target_sources(Engine00Interface INTERFACE
        include/Engine.hpp
        include/Engine/Config.hpp
        include/Engine/Action.hpp
        include/Engine/Actor.hpp
        include/Engine/Engine.hpp
        include/Engine/ActionCategory.hpp
        include/Engine/ActionInstance.hpp
        include/Engine/World.hpp
        include/Engine/Resource.hpp
        include/Engine/ResourcePtr.hpp
        include/Engine/GameClock.hpp
        include/Engine/DefaultBitmapHelpers.hpp

        include/Engine/Platform/Painter.hpp
        include/Engine/Platform/ResourceLoaderOptions.hpp
        include/Engine/Platform/ResourceManager.hpp
        include/Engine/Platform/ResourceLoader.hpp
        include/Engine/Platform/StreamFactory.hpp
        include/Engine/Platform/DrawableSurface.hpp
        include/Engine/Platform/InputSystem.hpp
        include/Engine/Platform/InputEvent.hpp
        include/Engine/Platform/Stream.hpp

        include/Engine/Math/SpacePartition.hpp
        include/Engine/Math/Vec2D.hpp
        include/Engine/Math/Rect.hpp
        include/Engine/Math/Color.hpp
        include/Engine/Math/AABB.hpp

        include/Engine/Detail/StringFormat.hpp
        include/Engine/Detail/TypeId.hpp
        include/Engine/Detail/PropertySet.hpp
        include/Engine/Detail/Property.hpp

        include/Engine/Resource/Map.hpp
        include/Engine/Resource/Palette.hpp
        include/Engine/Resource/Font.hpp
        include/Engine/Resource/DrawableResource.hpp
        include/Engine/Resource/Bitmap.hpp
        include/Engine/Resource/Sprite.hpp

        include/Engine/GUI/FontGlyph.hpp
        include/Engine/GUI/Menu.hpp
        include/Engine/GUI/Widget.hpp
        include/Engine/GUI/WorldWidget.hpp
        include/Engine/GUI/LabelWidget.hpp

        include/Engine/Logging/SourceLocation.hpp
        include/Engine/Logging/Logger.hpp

        include/Engine/Scripting/BoxedCast.hpp
        include/Engine/Scripting/BoxedValue.hpp
        include/Engine/Scripting/FunctionParams.hpp
        include/Engine/Scripting/ProxyFunction.hpp
        include/Engine/Scripting/ScriptEngine.hpp
        include/Engine/Scripting/detail/AttributeAccess.hpp
        include/Engine/Scripting/detail/CallUtils.hpp
        include/Engine/Scripting/detail/CastHelpers.hpp
        include/Engine/Scripting/detail/FunctionSignature.hpp
        include/Engine/Scripting/detail/HandleReturn.hpp
        include/Engine/Scripting/detail/NativeFunctionT.hpp
        include/Engine/Scripting/detail/Utils.hpp
)

if (DJGPP)
    message(STATUS "Building for MS-DOS Protected mode")
    add_subdirectory(src/Backend_DOS)
elseif (APPLE)
    message(STATUS "Building for Apple OSX")
    add_subdirectory(src/Backend_Apple)
else ()
    message(STATUS "Building for the SDL platform")
    add_subdirectory(src/Backend_SDL)
endif ()
