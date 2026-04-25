#include "path_utils.h"

namespace util {
bool IsSubPath(std::filesystem::path path, std::filesystem::path base) {
  // Приводим оба пути к каноничному виду (без . и ..)
  path = std::filesystem::weakly_canonical(path);
  base = std::filesystem::weakly_canonical(base);

  // Проверяем, что все компоненты base содержатся внутри path
  for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
    if (p == path.end() || *p != *b) {
      return false;
    }
  }
  return true;
}

std::string url_decode(const std::string_view value) {
  std::string result;
  result.reserve(value.length());

  for (size_t i = 0; i < value.length(); ++i) {
    if (value[i] == '%' && i + 2 < value.length()) {

      int hex_value = 0;
      for (int j = 1; j <= 2; ++j) {
        char c = value[i + j];
        if (c >= '0' && c <= '9') {
          hex_value = hex_value * 16 + (c - '0');
        } else if (c >= 'A' && c <= 'F') {
          hex_value = hex_value * 16 + (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f')
          hex_value = hex_value * 16 + (c - 'a' + 10);
        else {
          result += value[i];
          goto next_char;
        }
      }
      result += static_cast<char>(hex_value);

      i += 2;
    } else if (value[i] == '+') {
      result += ' ';
    } else {
      result += value[i];
    }

  next_char:;
  }

  return result;
}

} // namespace util