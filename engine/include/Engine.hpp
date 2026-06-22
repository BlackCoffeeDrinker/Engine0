#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#define NOT_COPYABLE(CLASS_NAME)                      \
  CLASS_NAME(const CLASS_NAME &) = delete;            \
  CLASS_NAME(CLASS_NAME &&) = delete;                 \
  CLASS_NAME &operator=(const CLASS_NAME &) = delete; \
  CLASS_NAME &operator=(CLASS_NAME &&) = delete

#include <Engine/Config.hpp>

#include <Engine/Detail/Property.hpp>
#include <Engine/Detail/PropertySet.hpp>
#include <Engine/Detail/StringFormat.hpp>
#include <Engine/Detail/TypeId.hpp>

#include <Engine/Logging/Logger.hpp>
#include <Engine/Logging/SourceLocation.hpp>

#include <Engine/Math/AABB.hpp>
#include <Engine/Math/Color.hpp>
#include <Engine/Math/Math.hpp>
#include <Engine/Math/Rect.hpp>
#include <Engine/Math/SpacePartition.hpp>
#include <Engine/Math/Vec2D.hpp>

#include <Engine/Action.hpp>
#include <Engine/ActionCategory.hpp>
#include <Engine/ActionInstance.hpp>
#include <Engine/Actor.hpp>
#include <Engine/DefaultBitmapHelpers.hpp>
#include <Engine/GameClock.hpp>
#include <Engine/Resource.hpp>
#include <Engine/ResourcePtr.hpp>
#include <Engine/World.hpp>


#include <Engine/Resource/Bitmap.hpp>
#include <Engine/Resource/Font.hpp>
#include <Engine/Resource/Map.hpp>
#include <Engine/Resource/Palette.hpp>
#include <Engine/Resource/Sprite.hpp>

#include <Engine/Platform/DrawableSurface.hpp>
#include <Engine/Platform/InputEvent.hpp>
#include <Engine/Platform/InputSystem.hpp>
#include <Engine/Platform/Painter.hpp>
#include <Engine/Platform/Stream.hpp>
#include <Engine/Platform/ResourceLoader.hpp>
#include <Engine/Platform/ResourceLoaderOptions.hpp>
#include <Engine/Platform/ResourceManager.hpp>
#include <Engine/Platform/StreamFactory.hpp>

#include <Engine/GUI/FontGlyph.hpp>
#include <Engine/GUI/LabelWidget.hpp>
#include <Engine/GUI/Menu.hpp>
#include <Engine/GUI/Widget.hpp>
#include <Engine/GUI/WorldWidget.hpp>


#include <Engine/Scripting/ScriptEngine.hpp>

#include <Engine/Engine.hpp>

#undef NOT_COPYABLE

namespace e00 {
/**
 * Must be called first, initializes the global variables and starts platform code
 * @return Any errors starting the platform
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
void Run(e00::Engine &engine);
}// namespace e00
