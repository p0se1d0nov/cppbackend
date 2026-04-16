#include <cassert>
#include <filesystem>

namespace fs = std::filesystem;

namespace path_utils {
bool IsSubPath(fs::path path, fs::path base);
std::string url_decode(const std::string_view value);
} // namespace path_utils