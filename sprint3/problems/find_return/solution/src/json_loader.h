#pragma once

#include <filesystem>
#include <unordered_map>
#include <boost/json.hpp>
#include "model.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path, bool random_spawn);

}  // namespace json_loader
