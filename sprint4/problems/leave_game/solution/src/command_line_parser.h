#pragma once

#include <optional>
#include <string>
#include <filesystem>

namespace command {

struct Args {
    std::optional<int> tick_period;
    std::string config_file;
    std::string www_root;
    bool random_spawn = false;
    bool help = false;
    std::optional<std::filesystem::path> state_file;
    std::optional<std::chrono::milliseconds> save_state_period;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]);

} // namespace
