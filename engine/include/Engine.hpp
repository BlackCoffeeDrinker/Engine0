#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <chrono>
#include <limits>
#include <memory>
#include <map>
#include <system_error>
#include <vector>
#include <iterator>
#include <type_traits>
#include <string_view>
#include <string>
#include <set>
#include <utility>
#include <list>

#define NOT_COPYABLE(CLASS_NAME) \
    CLASS_NAME (const CLASS_NAME &) = delete; \
    CLASS_NAME ( CLASS_NAME && ) = delete;    \
    CLASS_NAME & operator=( const CLASS_NAME & ) = delete; \
    CLASS_NAME & operator=( CLASS_NAME && ) = delete

#include <Engine/Config.hpp>

#include <Engine/Detail/Property.hpp>
#include <Engine/Detail/StringFormat.hpp>
#include <Engine/Detail/TypeId.hpp>
#include <Engine/Detail/Array.hpp>
#include <Engine/Detail/CircularBuffer.hpp>

#include <Engine/Logging/SourceLocation.hpp>
#include <Engine/Logging/Logger.hpp>

#include <Engine/Math/Vec2D.hpp>
#include <Engine/Math/Rect.hpp>
#include <Engine/Math/Color.hpp>
#include <Engine/Math/AABB.hpp>
#include <Engine/Math/SpacePartition.hpp>

#include <Engine/Resource.hpp>
#include <Engine/ResourcePtr.hpp>
#include <Engine/GameClock.hpp>
#include <Engine/ComponentRegistry.hpp>
#include <Engine/ActionCategory.hpp>
#include <Engine/Action.hpp>
#include <Engine/ActionInstance.hpp>
#include <Engine/Binding.hpp>
#include <Engine/InputSystem.hpp>
#include <Engine/InputEvent.hpp>
#include <Engine/Stream.hpp>

#include <Engine/Resource/Bitmap.hpp>
#include <Engine/Resource/Font.hpp>
#include <Engine/Resource/Sprite.hpp>
#include <Engine/Resource/Tileset.hpp>
#include <Engine/Resource/Map.hpp>

#include <Engine/Actor.hpp>
#include <Engine/World.hpp>

#include <Engine/GUI/Widget.hpp>
#include <Engine/GUI/Menu.hpp>
#include <Engine/GUI/FontGlyph.hpp>

#include <Engine/Scripting/ScriptEngine.hpp>

#include <Engine/Engine.hpp>

namespace e00 {
/**
 * Must be called first, initializes the global variables and starts platform code
 * @return error starting platform
 */
std::error_code Init();

/**
 * Called to clean up what was done in Init()
 */
void Exit();

/**
 * Run engine
 * @param engine the engine to execute
 */
void Run(e00::Engine& engine);
}

