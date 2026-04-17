#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "players_route.h"
#include "game.h"
#include "iroute.h"
#include "request_constant_values.h"

namespace Routes {

using namespace std::literals;
using namespace model;

http_handler::LogResponseData
PlayersRoute::operator()(const It begin, const It end, const Method method,
                         const Request &req) const {

  if (begin != end) {
    const auto api_version = this->routes_.find(std::string(*begin));
    if (api_version != this->routes_.end()) {
      const auto &game_route_ptr = api_version->second;
      return (*game_route_ptr)(begin, end, method, req);
    }
  }

  if (!(req.method() == http::verb::get || req.method() == http::verb::head)) {
    return method(http::status::method_not_allowed, INVALID_METHOD,
                  "GET, HEAD"sv);
  }

  auto token =
      this->game_.GetAuthorization().GetToken(method, req["Authorization"]);

  if (std::holds_alternative<std::string>(token)) {

    const auto session_ptr =
        this->game_.GetSessionByToken(std::get<std::string>(token));

    if (session_ptr) {

      const auto &players = session_ptr->GetPlayers();
      boost::json::object json_players{};
      for (const auto &player : players) {
        json_players.emplace(std::to_string(player.GetSessionId()),
                             boost::json::object{{"name", player.GetName()}});
      }

      return method(http::status::ok,
                    boost::json::parse(boost::json::serialize(json_players)),
                    ""sv);
    }

  } else {
    return std::get<http_handler::LogResponseData>(token);
  }

  return method(http::status::unauthorized, TOKEN_NOT_FOUND, ""sv);
}

} // namespace Routes