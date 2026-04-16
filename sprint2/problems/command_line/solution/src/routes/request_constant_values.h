#pragma once
#include "boost_json.h"

namespace Routes {
const inline static auto BAD_REQUEST =
    boost::json::parse(R"({"code": "badRequest", "message": "Bad request"})");

const inline static auto MAP_NOT_FOUND = boost::json::parse(
    R"({"code": "mapNotFound", "message": "Map not found"})");

const inline static auto INVALID_METHOD = boost::json::parse(
    R"({"code": "invalidMethod", "message": "Invalid method"})");

const inline static auto EMPTY_BODY = boost::json::parse(R"({})");

const inline static auto TOKEN_NOT_FOUND = boost::json::parse(
    R"({"code": "unknownToken", "message": "Player token has not been found"})");

const inline static auto FAILED_TICK = boost::json::parse(
    R"({"code": "invalidArgument", "message": "Failed to parse tick request JSON"})");

const inline static auto AUTH_HEADER_MISSING = boost::json::parse(
    R"({"code": "invalidToken", "message": "Authorization header is missing"})");

const inline static auto BEARER_MISSING = boost::json::parse(
    R"({"code": "invalidToken", "message": "Bearer is missing"})");

const inline static auto WRONG_MOVE_COMMAND = boost::json::parse(
    R"({"code": "invalidArgument", "message": "Wrong move command form request body"})");

const inline static auto NO_MOVE_COMMAND = boost::json::parse(
    R"({"code": "invalidArgument", "message": "No move command"})");

const inline static auto JOIN_GAME_ERROR = boost::json::parse(
    R"({"code": "invalidArgument", "message": "Join game request parse error"})");
const inline static auto NO_MAP_ID = boost::json::parse(
    R"({"code": "invalidArgument", "message": "Map id is missing"})");
const inline static auto NO_USER_NAME = boost::json::parse(
    R"({"code": "invalidArgument", "message": "User name is missing"})");
const inline static auto INVALID_NAME = boost::json::parse(
    R"({"code": "invalidArgument", "message": "Invalid name"})");

} // namespace Routes