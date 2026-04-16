#pragma once

#include "game.h"
#include "iroute.h"

namespace Routes {

class ActionRoute : public IRoute {

public:
  explicit ActionRoute(model::Game &game);

  ActionRoute(const ActionRoute &) = delete;
  ActionRoute &operator=(const ActionRoute &) = delete;
  http_handler::LogResponseData operator()(It begin, It end, Method method,
                                           const Request &req) const override;
};
} // namespace Routes