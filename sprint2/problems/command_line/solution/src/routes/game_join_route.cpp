#include "game_join_route.h"
#include "game.h"
#include "randomizer.h"
#include "request_constant_values.h"

namespace Routes {

using namespace std::literals;
using namespace model;

http_handler::LogResponseData
GameJoinRoute::operator()(const It begin, const It end, const Method method,
                          const Request &req) const {
  if (req.method() != http::verb::post) {
    return method(http::status::method_not_allowed, INVALID_METHOD, "POST"sv);
  }

  try {
    boost::json::object json_obj;
    try {
      json_obj = boost::json::parse(req.body()).as_object();
    } catch (const std::exception &ex) {
      return method(http::status::bad_request, JOIN_GAME_ERROR, ""sv);
    };

    const auto user_name_ptr = json_obj.if_contains("userName");
    const auto map_id = json_obj.if_contains("mapId");

    if (!map_id) {
      return method(http::status::bad_request, NO_MAP_ID, ""sv);
    }

    if (!user_name_ptr) {
      return method(http::status::bad_request, NO_USER_NAME, ""sv);
    }

    if (user_name_ptr->is_string() && map_id->is_string()) {
      const auto id = Map::Id(std::string(map_id->as_string()));
      const auto map_ptr = game_.FindMap(id);
      const auto user_name = std::string(user_name_ptr->as_string());

      if (user_name.empty()) {
        return method(http::status::bad_request, INVALID_NAME, ""sv);
      }

      if (!map_ptr) {
        return method(http::status::not_found, MAP_NOT_FOUND, ""sv);
      }

      const auto new_player_ptr = game_.AddPlayer(user_name, map_ptr);

      return method(http::status::ok,
                    boost::json::parse(
                        "{\"authToken\":\""s + new_player_ptr->GetToken() +
                        "\",\"playerId\":"s +
                        std::to_string(new_player_ptr->GetSessionId()) + "}"s),
                    ""sv);
    }

    return method(http::status::bad_request, BAD_REQUEST, ""sv);

  } catch (...) {
    return method(http::status::bad_request, BAD_REQUEST, ""sv);
  }
};

} // namespace Routes