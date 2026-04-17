#pragma once

#include "game.h"
#include "iroute.h"

namespace Routes {

class GameJoinRoute : public IRoute {

public:
  explicit GameJoinRoute(model::Game &game) : IRoute{game} {};

  GameJoinRoute(const GameJoinRoute &) = delete;
  GameJoinRoute &operator=(const GameJoinRoute &) = delete;

  http_handler::LogResponseData operator()(const It begin, const It end,
                                           const Method method,
                                           const Request &req) const;

private:
};
} // namespace Routes