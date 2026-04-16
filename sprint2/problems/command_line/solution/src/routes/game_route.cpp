#include "game_route.h"
#include "game.h"
#include "game_join_route.h"
#include "game_state_route.h"
#include "iroute.h"
#include "player_route.h"
#include "players_route.h"
#include "request_constant_values.h"
#include "tick_route.h"

#include <string_view>

namespace Routes {

using namespace std::literals;
using namespace model;

GameRoute::GameRoute(Game &game) : IRoute(game) {
  this->routes_.emplace("join", std::make_shared<GameJoinRoute>(game));
  this->routes_.emplace("players", std::make_shared<PlayersRoute>(game));
  this->routes_.emplace("player", std::make_shared<PlayerRoute>(game));
  this->routes_.emplace("state", std::make_shared<GameStateRoute>(game));
  this->routes_.emplace("tick"s, std::make_shared<TickRoute>(game));
};

http_handler::LogResponseData GameRoute::operator()(It begin, It end,
                                                    Method method,
                                                    const Request &req) const {

  if (begin == end) {
    return method(http::status::bad_request, BAD_REQUEST, ""sv);
  }

  const auto api_version = this->routes_.find(std::string(*begin));
  if (api_version != this->routes_.end()) {
    const auto &game_route_ptr = api_version->second;
    return (*game_route_ptr)(begin + 1, end, method, req);
  }

  return method(http::status::bad_request, BAD_REQUEST, ""sv);
}

} // namespace Routes