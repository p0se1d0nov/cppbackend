#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "log_response_data.h"
#include "route_types.h"

#include <string>
#include <variant>

namespace services {

using namespace std::literals;

class Authorization {
public:
  explicit Authorization() {}

  Authorization(const Authorization &) = delete;
  Authorization &operator=(const Authorization &) = delete;

public:
  std::variant<http_handler::LogResponseData, std::string>
  GetToken(const Routes::Method method,
           const std::string_view authorization) const;

private:
};
} // namespace services