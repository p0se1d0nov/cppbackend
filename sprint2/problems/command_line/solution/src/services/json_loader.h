#pragma once

#include "game.h"

#include <boost/asio/io_context.hpp>
#include <boost/program_options.hpp>

#include <filesystem>

namespace json_loader {

model::Game LoadGame(const std::filesystem::path &json_path,
                     boost::asio::io_context &ioc,
                     boost::optional<uint64_t> tick_period,
                     bool randomize_spawn_point);

} // namespace json_loader