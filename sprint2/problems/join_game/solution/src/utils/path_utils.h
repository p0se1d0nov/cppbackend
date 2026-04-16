#include <cassert>
#include <filesystem>

namespace util {
bool IsSubPath(std::filesystem::path path, std::filesystem::path base);
std::string url_decode(const std::string_view value);
} // namespace util