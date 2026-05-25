#pragma once
#include "http_server.h"
#include "model.h"
#include "players.h"
#include <boost/json.hpp>
#include <boost/json/error.hpp>
#include <boost/asio/strand.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iterator>   // для transform
#include "app.h"

namespace fs = std::filesystem;
namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

// Вспомогательные функции (объявления)
std::string ToLower(std::string s);
std::string UrlDecode(std::string_view target);
std::string GetMimeType(const std::filesystem::path path);
bool IsSubPath(fs::path path, fs::path base);
boost::json::value SerializeMapShort(const model::Map& map);
boost::json::object SerializeMapFull(const model::Map& map);

// Шаблонные функции для формирования ответов
template <typename Send>
inline void MakeJsonResponse(Send&& send, http::status status, std::string_view body, unsigned version = 11) {
    http::response<http::string_body> res{status, version};
    res.set(http::field::content_type, "application/json");
    res.set(http::field::cache_control, "no-cache");
    res.body() = body;
    res.prepare_payload();
    send(std::move(res));
}

template <typename Send>
inline void MakePlainTextResponse(Send&& send, http::status status, std::string_view text, unsigned version = 11) {
    http::response<http::string_body> res{status, version};
    res.set(http::field::content_type, "text/plain");
    res.body() = text;
    res.prepare_payload();
    send(std::move(res));
}

template <typename Send>
inline void SendHeadResponse(Send&& send, http::status status, unsigned version,
                             std::string_view content_type, std::uint64_t content_length = 0) {
    http::response<http::empty_body> res{status, version};
    res.set(http::field::content_type, content_type);
    res.set(http::field::content_length, std::to_string(content_length));
    send(std::move(res));
}

template <typename Send>
inline void MakeErrorResponse(Send&& send, http::status status, std::string_view code, std::string_view message, unsigned version = 11) {
    boost::json::object err;
    err["code"] = std::string(code);
    err["message"] = std::string(message);
    http::response<http::string_body> res{status, version};
    res.set(http::field::content_type, "application/json");
    res.set(http::field::cache_control, "no-cache");
    res.body() = boost::json::serialize(err);
    res.prepare_payload();
    send(std::move(res));
}

template <typename Send>
inline void SendMethodNotAllowed(Send&& send, std::string_view allowed_methods, std::string_view message, unsigned version) {
    http::response<http::string_body> res{http::status::method_not_allowed, version};
    res.set(http::field::content_type, "application/json");
    res.set(http::field::allow, allowed_methods);
    res.set(http::field::cache_control, "no-cache");
    boost::json::object err;
    err["code"] = "invalidMethod";
    err["message"] = std::string(message);
    res.body() = boost::json::serialize(err);
    res.prepare_payload();
    send(std::move(res));
}

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit RequestHandler(app::Application& app,
                            fs::path static_dir,
                            Strand api_strand, bool auto_tick_enabled)
        : static_dir_(fs::weakly_canonical(static_dir))
        , api_strand_(api_strand)
        , app_(app)
        , auto_tick_enabled_ (auto_tick_enabled)
    {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        try {
            std::string_view target = req.target();
            if (target.starts_with("/api/")) {
                unsigned version = req.version();
                auto handler = [self = shared_from_this(),
                                req = std::move(req),
                                send = std::forward<Send>(send),
                                version]() mutable {
                    try {
                        self->HandleApiRequest(std::move(req), std::move(send));
                    } catch (const std::exception& e) {
                        MakeErrorResponse(std::move(send),
                                          http::status::internal_server_error,
                                          "internalError", e.what(), version);
                    }
                    // FIX: убран catch (...)
                };
                return net::dispatch(api_strand_, std::move(handler));
            }
            HandleStaticRequest(std::move(req), std::forward<Send>(send));
        } catch (const std::exception& e) {
            MakeErrorResponse(std::forward<Send>(send),
                              http::status::internal_server_error,
                              "internalError", e.what(), req.version());
        }
        // FIX: убран catch (...)
    }

private:
    template <typename Body, typename Allocator, typename Send>
    void HandleApiRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void HandleJoinRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void HandlePlayersRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void HandleStaticRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void HandleGameStateRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void HandlePlayerActionRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void HandleGameTick(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void HandleMapRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);


    fs::path static_dir_;
    Strand api_strand_;
    app::Application& app_;
    bool auto_tick_enabled_;
};


template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandleApiRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    std::string_view target = req.target();
    constexpr std::string_view V1_PREFIX = "/api/v1";

    if (!target.starts_with(V1_PREFIX)) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "badRequest",
                                 "Bad API version",
                                 req.version());
    }
    target.remove_prefix(V1_PREFIX.size());

    if (target == "/maps") {
        if (req.method() != http::verb::get) {
            return SendMethodNotAllowed(std::forward<Send>(send), "GET",
                                        "Only GET method is expected", req.version());
        }
        boost::json::array arr;
        for (const auto& map : app_.GetMaps()) {
            arr.push_back(SerializeMapShort(map));
        }
        std::string body = boost::json::serialize(arr);
        return MakeJsonResponse(std::forward<decltype(send)>(send),
                                http::status::ok, body, req.version());
    }

    if (target.starts_with("/maps/")) {
        HandleMapRequest(std::move(req), std::forward<Send>(send));
        return;
    }

    if (target == "/game/join") {
        if (req.method() != http::verb::post) {
            return SendMethodNotAllowed(std::forward<Send>(send), "POST",
                                        "Only POST method is expected", req.version());
        }
        HandleJoinRequest(std::move(req), std::forward<Send>(send));
    } else if (target == "/game/players") {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            return SendMethodNotAllowed(std::forward<Send>(send), "GET, HEAD",
                                        "Invalid method", req.version());
        }
        HandlePlayersRequest(std::move(req), std::forward<Send>(send));
    } else if (target == "/game/state") {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            return SendMethodNotAllowed(std::forward<Send>(send), "GET, HEAD",
                                        "Invalid method", req.version());
        }
        HandleGameStateRequest(std::move(req), std::forward<Send>(send));
    } else if (target == "/game/player/action") {
        if (req.method() != http::verb::post) {
            return SendMethodNotAllowed(std::forward<Send>(send), "POST",
                                        "Invalid method", req.version());
        }
        HandlePlayerActionRequest(std::move(req), std::forward<Send>(send));
    } else if (target == "/game/tick") {
        if (req.method() != http::verb::post) {
            return SendMethodNotAllowed(std::forward<Send>(send), "POST",
                                        "Invalid method", req.version());
        }
        HandleGameTick(std::move(req), std::forward<Send>(send));
    } else {
        MakeErrorResponse(std::forward<Send>(send),
                          http::status::bad_request,
                          "badRequest",
                          "Invalid endpoint",
                          req.version());
        return;
    }
}

template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandleMapRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    std::string_view target = req.target();
    constexpr std::string_view V1_PREFIX = "/api/v1";
    if (!target.starts_with(V1_PREFIX)) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "badRequest",
                                 "Bad API version",
                                 req.version());
    }
    target.remove_prefix(V1_PREFIX.size());

    if (!target.starts_with("/maps/")) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "badRequest",
                                 "Invalid map endpoint",
                                 req.version());
    }

    if (req.method() != http::verb::get && req.method() != http::verb::head) {
        return SendMethodNotAllowed(std::forward<Send>(send), "GET, HEAD",
                                    "Only GET and HEAD are expected", req.version());
    }

    std::string_view id_str = target.substr(6); // после "/maps/"
    if (id_str.empty()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "badRequest",
                                 "Missing map id",
                                 req.version());
    }

    const auto* map = app_.FindMap(std::string(id_str));
    if (!map) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::not_found,
                                 "mapNotFound",
                                 "Map not found",
                                 req.version());
    }

    boost::json::object obj = SerializeMapFull(*map);
    obj["lootTypes"] = app_.GetLootTypesForMap(map->GetId());

    std::string body = boost::json::serialize(obj);
    if (req.method() == http::verb::head) {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set(http::field::cache_control, "no-cache");
        res.content_length(body.size());
        send(std::move(res));
    } else {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set(http::field::cache_control, "no-cache");
        res.body() = std::move(body);
        res.prepare_payload();
        send(std::move(res));
    }
}

template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandleGameTick(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    if (auto_tick_enabled_) {
        MakeErrorResponse(std::forward<Send>(send),
                          http::status::bad_request,
                          "badRequest",
                          "Invalid endpoint",
                          req.version());
        return;
    }
    boost::system::error_code ec;
    auto jv = boost::json::parse(req.body(), ec);

    if (ec || !jv.is_object()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Failed to parse tick request JSON.",
                                 req.version());
    }

    auto content_type_it = req.find(http::field::content_type);

    if (content_type_it == req.end()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid content type",
                                 req.version());
    }

    std::string_view content_type = content_type_it->value();
    if (content_type != "application/json") {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid content type",
                                 req.version());
    }

    auto& obj = jv.as_object();
    if (!obj.contains("timeDelta")) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Missing timeDelta field",
                                 req.version());
    }
    if (!obj["timeDelta"].is_int64()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "timeDelta must be an integer",
                                 req.version());
    }
    int64_t delta_ms = obj["timeDelta"].as_int64();
    if (delta_ms < 0) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "timeDelta must be non-negative",
                                 req.version());
    }

    try {
        app_.UpdateGameState(std::chrono::milliseconds(delta_ms));
    } catch (const std::exception& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::internal_server_error,
                                 "internalError",
                                 e.what(),
                                 req.version());
    }

    std::string empty_body = "{}";
    if (req.method() == http::verb::head) {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set(http::field::cache_control, "no-cache");
        res.content_length(empty_body.size());
        send(std::move(res));
    } else {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set(http::field::cache_control, "no-cache");
        res.body() = empty_body;
        res.prepare_payload();
        send(std::move(res));
    }
}

template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandlePlayerActionRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    auto auth_header = req.find(http::field::authorization);
    if (auth_header == req.end()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Authorization header is required",
                                 req.version());
    }

    auto content_type_it = req.find(http::field::content_type);

    if (content_type_it == req.end()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid content type",
                                 req.version());
    }

    std::string_view content_type = content_type_it->value();
    if (content_type != "application/json") {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid content type",
                                 req.version());
    }

    std::string auth_value = auth_header->value();
    constexpr std::string_view bearer_prefix = "Bearer ";
    if (!auth_value.starts_with(bearer_prefix)) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Invalid authorization header format",
                                 req.version());
    }
    std::string token = auth_value.substr(bearer_prefix.size());

    if (token.size() != 32 || token.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Invalid token format",
                                 req.version());
    }
    boost::system::error_code ec;
    boost::json::value jv;
    jv = boost::json::parse(req.body(), ec);
    if (ec || !jv.is_object()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Action request parse error",
                                 req.version());
    }
    if (!jv.as_object().contains("move")) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Failed to find {move}",
                                 req.version());
    }
    std::string str_dir = jv.at("move").as_string().c_str();
    if (str_dir.size() > 1) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid Direction",
                                 req.version());
    }
    try {
        app_.SetPlayerSpeed(token, str_dir);
        boost::json::object resp;
        std::string body = boost::json::serialize(resp);
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set(http::field::cache_control, "no-cache");
        res.body() = std::move(body);
        res.prepare_payload();
        send(std::move(res));
    } catch (const app::InvalidArgument& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid Direction",
                                 req.version());
    } catch (const std::runtime_error& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "unknownToken",
                                 e.what(),
                                 req.version());
    } catch (const std::exception& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::internal_server_error,
                                 "internalError",
                                 e.what(),
                                 req.version());
    }
}

template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandleJoinRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    boost::system::error_code ec;
    boost::json::value jv;
    jv = boost::json::parse(req.body(), ec);

    if (ec || !jv.is_object()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Join game request parse error",
                                 req.version());
    }
    auto& obj = jv.as_object();
    if (!obj.contains("userName") || !obj["userName"].is_string()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid name",
                                 req.version());
    }
    std::string user_name = obj["userName"].as_string().c_str();
    if (user_name.empty()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid name",
                                 req.version());
    }
    if (!obj.contains("mapId") || !obj["mapId"].is_string()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::bad_request,
                                 "invalidArgument",
                                 "Invalid mapId",
                                 req.version());
    }
    std::string map_id = obj["mapId"].as_string().c_str();

    try {
        auto result = app_.JoinGame(user_name, map_id);
        boost::json::object resp;
        resp["authToken"] = result.token;
        resp["playerId"] = result.player_id;
        std::string body = boost::json::serialize(resp);
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set(http::field::cache_control, "no-cache");
        res.body() = std::move(body);
        res.prepare_payload();
        send(std::move(res));
    } catch (const std::invalid_argument& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::not_found,
                                 "mapNotFound",
                                 e.what(),
                                 req.version());
    } catch (const std::exception& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::internal_server_error,
                                 "internalError",
                                 e.what(),
                                 req.version());
    }
}

template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandlePlayersRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    auto auth_header = req.find(http::field::authorization);
    if (auth_header == req.end()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Authorization header is missing",
                                 req.version());
    }
    std::string auth_value = auth_header->value();
    constexpr std::string_view bearer_prefix = "Bearer ";
    if (!auth_value.starts_with(bearer_prefix)) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Invalid authorization header format",
                                 req.version());
    }
    std::string token = auth_value.substr(bearer_prefix.size());

    if (token.size() != 32 || token.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Invalid token format",
                                 req.version());
    }

    try {
        auto players = app_.GetPlayers(token);
        boost::json::object players_obj;
        for (const auto& [player_id, name] : players) {
            players_obj[std::to_string(player_id)] = boost::json::object{{"name", name}};
        }
        std::string body = boost::json::serialize(players_obj);

        if (req.method() == http::verb::head) {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.set(http::field::cache_control, "no-cache");
            res.content_length(body.size());
            send(std::move(res));
        } else {
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.set(http::field::cache_control, "no-cache");
            res.body() = std::move(body);
            res.prepare_payload();
            send(std::move(res));
        }
    } catch (const std::runtime_error& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "unknownToken",
                                 e.what(),
                                 req.version());
    } catch (const std::exception& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::internal_server_error,
                                 "internalError",
                                 e.what(),
                                 req.version());
    }
}

template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandleGameStateRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    auto auth_header = req.find(http::field::authorization);
    if (auth_header == req.end()) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Authorization header is missing",
                                 req.version());
    }
    std::string auth_value = auth_header->value();
    constexpr std::string_view bearer_prefix = "Bearer ";
    if (!auth_value.starts_with(bearer_prefix)) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Invalid authorization header format",
                                 req.version());
    }
    std::string token = auth_value.substr(bearer_prefix.size());

    if (token.size() != 32 || token.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "invalidToken",
                                 "Invalid token format",
                                 req.version());
    }

    try {
        auto states = app_.GetGameState(token);
        boost::json::object players_obj;
        for (const auto& state : states) {
            boost::json::object player_state;
            player_state["pos"] = boost::json::array{state.x, state.y};
            player_state["speed"] = boost::json::array{state.vx, state.vy};
            player_state["score"] = state.score;

            // FIX: использование std::transform вместо ручного цикла
            boost::json::array bag_array;
            std::transform(state.bag.begin(), state.bag.end(), std::back_inserter(bag_array),
                           [](const model::Loot& loot) {
                               return boost::json::object{{"id", loot.id}, {"type", loot.type}};
                           });
            player_state["bag"] = std::move(bag_array);

            std::string dir_str;
            switch (state.dir) {
            case model::Direction::NORTH: dir_str = "U"; break;
            case model::Direction::SOUTH: dir_str = "D"; break;
            case model::Direction::WEST:  dir_str = "L"; break;
            case model::Direction::EAST:  dir_str = "R"; break;
            }
            player_state["dir"] = dir_str;
            players_obj[std::to_string(state.player_id)] = std::move(player_state);
        }

        auto &loot = app_.GetLoot(token);
        boost::json::object lost_objects_obj;
        for (const auto& item : loot) {
            boost::json::object obj;
            obj["type"] = item.type;
            obj["pos"] = boost::json::array{item.x, item.y};
            lost_objects_obj[std::to_string(item.id)] = std::move(obj);
        }

        boost::json::object response;
        response["players"] = std::move(players_obj);
        response["lostObjects"] = std::move(lost_objects_obj);
        std::string body = boost::json::serialize(response);

        if (req.method() == http::verb::head) {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.set(http::field::cache_control, "no-cache");
            res.content_length(body.size());
            send(std::move(res));
        } else {
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.set(http::field::cache_control, "no-cache");
            res.body() = std::move(body);
            res.prepare_payload();
            send(std::move(res));
        }
    } catch (const std::runtime_error& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::unauthorized,
                                 "unknownToken",
                                 e.what(),
                                 req.version());
    } catch (const std::exception& e) {
        return MakeErrorResponse(std::forward<Send>(send),
                                 http::status::internal_server_error,
                                 "internalError",
                                 e.what(),
                                 req.version());
    }
}

template <typename Body, typename Allocator, typename Send>
void RequestHandler::HandleStaticRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    try {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            return MakeErrorResponse(std::forward<Send>(send),
                                     http::status::method_not_allowed,
                                     "methodNotAllowed",
                                     "Only GET and HEAD are supported for static files",
                                     req.version());
        }
        bool is_head = (req.method() == http::verb::head);
        std::string decoded_target = UrlDecode(req.target());
        fs::path rel_path;
        if (!decoded_target.empty() && decoded_target[0] == '/') {
            rel_path = decoded_target.substr(1);
        } else {
            rel_path = decoded_target;
        }
        fs::path full_path = static_dir_ / rel_path;
        if (!IsSubPath(full_path, static_dir_)) {
            if (is_head) {
                return SendHeadResponse(std::forward<Send>(send), http::status::bad_request,
                                        req.version(), "text/plain");
            } else {
                return MakePlainTextResponse(std::forward<Send>(send),
                                             http::status::bad_request,
                                             "Invalid path", req.version());
            }
        }
        std::error_code ec;
        fs::file_status status = fs::status(full_path, ec);
        if (ec) {
            if (is_head) {
                return SendHeadResponse(std::forward<Send>(send), http::status::not_found,
                                        req.version(), "text/plain");
            }
            return MakePlainTextResponse(std::forward<Send>(send),
                                         http::status::not_found,
                                         "File not found", req.version());
        }
        if (fs::is_directory(status)) {
            full_path /= "index.html";
            status = fs::status(full_path, ec);
            if (ec || !fs::is_regular_file(status)) {
                if (is_head) {
                    return SendHeadResponse(std::forward<Send>(send), http::status::not_found,
                                            req.version(), "text/plain");
                } else {
                    return MakePlainTextResponse(std::forward<Send>(send),
                                                 http::status::not_found,
                                                 "File not found", req.version());
                }
            }
        } else if (!fs::is_regular_file(status)) {
            if (is_head) {
                return SendHeadResponse(std::forward<Send>(send), http::status::not_found,
                                        req.version(), "text/plain");
            } else {
                return MakePlainTextResponse(std::forward<Send>(send),
                                             http::status::not_found,
                                             "File not found", req.version());
            }
        }
        uintmax_t file_size = fs::file_size(full_path, ec);
        if (ec) {
            if (is_head) {
                return SendHeadResponse(std::forward<Send>(send), http::status::not_found,
                                        req.version(), "text/plain");
            } else {
                return MakePlainTextResponse(std::forward<Send>(send),
                                             http::status::not_found,
                                             "File not found", req.version());
            }
        }
        std::string mime = GetMimeType(full_path);
        if (is_head) {
            SendHeadResponse(std::forward<Send>(send), http::status::ok,
                             req.version(), mime, file_size);
        } else {
            std::ifstream file(full_path, std::ios::binary);
            if (!file) {
                return MakePlainTextResponse(std::forward<Send>(send),
                                             http::status::not_found,
                                             "File not found", req.version());
            }
            std::string body;
            try {
                body.resize(file_size);
                file.read(body.data(), file_size);
                if (!file) {
                    throw std::runtime_error("Failed to read file");
                }
            } catch (const std::system_error& e) {
                return MakePlainTextResponse(std::forward<Send>(send),
                                             http::status::internal_server_error,
                                             "File system error", req.version());
            }
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, mime);
            res.body() = std::move(body);
            res.prepare_payload();
            send(std::move(res));
        }
    } catch (const std::exception& e) {
        MakeErrorResponse(std::forward<Send>(send),
                          http::status::internal_server_error,
                          "internalError", e.what(), req.version());
    }
}

} // namespace http_handler
