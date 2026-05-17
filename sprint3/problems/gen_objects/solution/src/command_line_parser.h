#pragma once

#include <optional>
#include <string>

namespace command {

struct Args {
    std::optional<int> tick_period;
    std::string config_file;
    std::string www_root;
    bool random_spawn = false;
    bool help = false;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]);

} // namespace
