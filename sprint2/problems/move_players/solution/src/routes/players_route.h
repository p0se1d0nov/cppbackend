#pragma once

#include "game.h"
#include "iroute.h"

namespace Routes {

class PlayersRoute : public IRoute {

public:
  explicit PlayersRoute(model::Game &game) : IRoute{game} {};

  PlayersRoute(const PlayersRoute &) = delete;
  PlayersRoute &operator=(const PlayersRoute &) = delete;

  http_handler::LogResponseData operator()(const It begin, const It end,
                                           const Method method,
                                           const Request &req) const override;
};
} // namespace Routes