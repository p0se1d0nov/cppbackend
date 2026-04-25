#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "action_route.h"
#include "details.h"
#include "game.h"
#include "game_state_route.h"
#include "iroute.h"
#include "request_constant_values.h"

#include <string_view>

namespace Routes {

using namespace std::literals;
using namespace model;

ActionRoute::ActionRoute(Game &game) : IRoute(game) {};

http_handler::LogResponseData
ActionRoute::operator()(It begin, It end, Method method,
                        const Request &req) const {

  if (req.method() != http::verb::post) {
    return method(http::status::method_not_allowed, INVALID_METHOD, "POST"sv);
  }

  auto token =
      this->game_.GetAuthorization().GetToken(method, req["Authorization"]);

  if (std::holds_alternative<std::string>(token)) {
    const auto session_ptr =
        this->game_.GetSessionByToken(std::get<std::string>(token));
    const auto player_ptr =
        session_ptr->GetPlayerByToken(std::get<std::string>(token));
    if (!player_ptr) {
      return method(http::status::unauthorized, TOKEN_NOT_FOUND, ""sv);
    }

    boost::json::object json_obj;
    try {
      json_obj = boost::json::parse(req.body()).as_object();
    } catch (const std::exception &ex) {
      return method(http::status::bad_request, WRONG_MOVE_COMMAND, ""sv);
    };

    const auto move_command_ptr = json_obj.if_contains("move");

    if (!move_command_ptr) {
      return method(http::status::bad_request, NO_MOVE_COMMAND, ""sv);
    }
    const auto move_command = move_command_ptr->as_string();

    const auto dog = player_ptr->GetCurrentDog();

    auto direction = detail::Direction::STOP;

    if (move_command == "U"s) {
      direction = detail::Direction::NORTH;
    };
    if (move_command == "R"s) {
      direction = detail::Direction::EAST;
    };
    if (move_command == "D"s) {
      direction = detail::Direction::SOUTH;
    };
    if (move_command == "L"s) {
      direction = detail::Direction::WEST;
    };

    boost::asio::dispatch(session_ptr->GetStrand(),
                          [dog, direction]() { dog->SetDirection(direction); });

    return method(http::status::ok, EMPTY_BODY, ""sv);

  } else {
    return std::get<http_handler::LogResponseData>(token);
  }

  return method(http::status::bad_request, BAD_REQUEST, ""sv);
}

} // namespace Routes