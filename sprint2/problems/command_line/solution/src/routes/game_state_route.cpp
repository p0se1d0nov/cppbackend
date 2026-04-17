#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "game_state_route.h"
#include "details.h"
#include "game.h"
#include "log_response_data.h"
#include "request_constant_values.h"

#include <boost/json/array.hpp>

namespace Routes {

using namespace std::literals;
using namespace model;

http_handler::LogResponseData
GameStateRoute::operator()(It begin, It end, Method method,
                           const Request &req) const {
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

      boost::json::object json_players{};
      for (const auto &player_ptr : session_ptr->GetPlayers()) {
        boost::json::object json_dog_state{};
        const auto [coordinate, _, direction, direction_speed] =
            player_ptr.GetCurrentDog()->GetState();
        const auto [x, y] = coordinate;
        const auto [xx, yy] = direction_speed;
        const auto dir = direction;
        json_dog_state["pos"s] = boost::json::array({x, y});
        json_dog_state["speed"s] = boost::json::array({xx, yy});
        switch (dir) {
        case detail::Direction::NORTH:
          json_dog_state["dir"s] = "U"s;
          break;
        case detail::Direction::EAST:
          json_dog_state["dir"s] = "R"s;
          break;
        case detail::Direction::SOUTH:
          json_dog_state["dir"s] = "D"s;
          break;
        case detail::Direction::WEST:
          json_dog_state["dir"s] = "L"s;
          break;
        default:
          json_dog_state["dir"s] = ""s;
        }
        json_players.emplace(std::to_string(player_ptr.GetSessionId()),
                             json_dog_state);
      }

      boost::json::object json_result{};
      json_result.emplace("players"s, json_players);
      return method(http::status::ok,
                    boost::json::parse(boost::json::serialize(json_result)),
                    ""sv);
    }
  } else {
    return std::get<http_handler::LogResponseData>(token);
  }

  return method(http::status::unauthorized, TOKEN_NOT_FOUND, ""sv);
}

} // namespace Routes