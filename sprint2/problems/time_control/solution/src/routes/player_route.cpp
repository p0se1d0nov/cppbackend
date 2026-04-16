#include "player_route.h"
#include "iroute.h"
#include "request_constant_values.h"

namespace Routes {

using namespace std::literals;
using namespace model;

http_handler::LogResponseData
PlayerRoute::operator()(const It begin, const It end, const Method method,
                        const Request &req) const {

  if (begin != end) {
    const auto api_version = this->routes_.find(std::string(*begin));
    if (api_version != this->routes_.end()) {
      const auto &game_route_ptr = api_version->second;
      return (*game_route_ptr)(begin, end, method, req);
    }
  }

  return method(http::status::unauthorized, TOKEN_NOT_FOUND, ""sv);
}

} // namespace Routes