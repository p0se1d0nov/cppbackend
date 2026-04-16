#pragma once

#include "game.h"
#include "iroute.h"

namespace Routes {

class GameRoute : public IRoute {

public:
  explicit GameRoute(model::Game &game);

  GameRoute(const GameRoute &) = delete;
  GameRoute &operator=(const GameRoute &) = delete;
  http_handler::LogResponseData operator()(It begin, It end, Method method,
                                           const Request &req) const override;
};
} // namespace Routes