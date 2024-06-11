#pragma once

#include <Platform.hpp>

#include "EngineData.hpp"
#include "EngineError.hpp"

#include "IniParser.hpp"

#include "Resource/ResourceManager.hpp"

namespace e00 {
const std::unique_ptr<ResourceManager> &GlobalResourceManager();
}
