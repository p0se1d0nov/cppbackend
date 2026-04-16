#pragma once
#include "action_route.h"
#include "game.h"
#include "iroute.h"

namespace Routes {

using namespace std::literals;
class PlayerRoute : public IRoute {

public:
  explicit PlayerRoute(model::Game &game) : IRoute{game} {
    this->routes_.emplace("action"s, std::make_shared<ActionRoute>(game));
  };

  PlayerRoute(const PlayerRoute &) = delete;
  PlayerRoute &operator=(const PlayerRoute &) = delete;

  http_handler::LogResponseData operator()(const It begin, const It end,
                                           const Method method,
                                           const Request &req) const override;
};
} // namespace Routes