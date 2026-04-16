#pragma once
#include <string_view>

namespace http_handler {

struct LogResponseData {
  unsigned int code;
  long int response_time;
  std::string_view content_type;
};
} // namespace http_handler