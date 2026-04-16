#pragma once

#include "game.h"
#include "log_response_data.h"
#include "route_types.h"
#include <unordered_map>

namespace Routes {

class IRoute {
public:
  IRoute(model::Game &game) : game_{game} {};
  virtual http_handler::LogResponseData
  operator()(It begin, It end, Method method, const Request &req) const = 0;
  virtual ~IRoute() = default;

protected:
  model::Game &game_;
  std::unordered_map<std::string, std::shared_ptr<IRoute>> routes_;
};

} // namespace Routes