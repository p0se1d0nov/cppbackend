#include "api_handler.h"
#include "api_versions_route.h"
#include "request_constant_values.h"

namespace Routes {
ApiHandler::ApiHandler(model::Game &game) : IRoute(game) {
  using namespace std::literals;
  this->routes_.emplace("v1"s, std::make_shared<ApiVersionsRoute>(game));
};

http_handler::LogResponseData ApiHandler::operator()(const It begin,
                                                     const It end,
                                                     const Method method,
                                                     const Request &req) const {
  using namespace std::literals;
  if (begin == end) {
    return method(http::status::bad_request, BAD_REQUEST, ""sv);
  }

  const auto api_version = this->routes_.find(std::string(*begin));
  if (api_version != this->routes_.end()) {
    const auto &game_route_ptr = api_version->second;
    return (*game_route_ptr)(begin + 1, end, method, req);
  }
  return method(http::status::bad_request, BAD_REQUEST, ""sv);
};
} // namespace Routes