#include "authorization.h"
#include "request_constant_values.h"

#include <string>
#include <variant>

namespace services {
std::variant<http_handler::LogResponseData, std::string>
Authorization::GetToken(const Routes::Method method,
                        const std::string_view authorization) const {

  if (authorization.empty()) {
    return method(boost::beast::http::status::unauthorized,
                  Routes::AUTH_HEADER_MISSING, ""s);
  }

  const auto is_bearer = authorization.find("Bearer "s) == 0;
  if (!is_bearer) {
    return method(boost::beast::http::status::unauthorized,
                  Routes::BEARER_MISSING, ""s);
  }

  size_t pos = authorization.find(' ');
  if (pos != std::string::npos) {

    const auto token = authorization.substr(pos + 1);

    if (token.size() > 32 || token.size() < 32) {
      return method(boost::beast::http::status::unauthorized,
                    Routes::AUTH_HEADER_MISSING, ""s);
    }

    return std::string(token);
  };
  return method(boost::beast::http::status::unauthorized,
                Routes::TOKEN_NOT_FOUND, ""s);
};
} // namespace services