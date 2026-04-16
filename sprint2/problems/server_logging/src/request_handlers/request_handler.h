#pragma once
#include <utility>
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/algorithm/string.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <string_view>
#include <vector>
#include <boost/beast/http/file_body.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include "http_server.h"
#include "model.h"
#include "path_utils.h"

#include <iostream>

#include "content_type.h"

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;

using namespace http;
using namespace std::literals;
using namespace model;
template <typename Body, typename Allocator>
using BasicRequest = request<Body, basic_fields<Allocator>>;

const std::vector<std::string_view> GetSplitUrlMembers(std::string_view view);

class RequestHandler {
 public:
  // Запрос, тело которого представлено в виде строки
  using StringRequest = request<string_body>;
  // Ответ, тело которого представлено в виде строки
  using StringResponse = response<string_body>;
  using FileResponse = response<file_body>;

  struct LogResponseData {
    unsigned int code;
    long int response_time;
    std::string_view content_type;
  };

 public:
  explicit RequestHandler(Game& game, std::string static_catalog)
      : game_{game}, static_catalog_{std::move(static_catalog)} {}

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  RequestHandler(RequestHandler&& handler)
      : game_{handler.game_},
        static_catalog_(std::move(handler.static_catalog_)),
        target_path_(std::move(handler.target_path_)) {};

  RequestHandler& operator=(RequestHandler&& handler) {
    if (&handler == this) {
      return *this;
    }

    std::swap(game_, handler.game_);
    std::swap(static_catalog_, handler.static_catalog_);
    std::swap(target_path_, handler.target_path_);
    return *this;
  };

  template <typename Body, typename Allocator, typename Send>
  LogResponseData operator()(BasicRequest<Body, Allocator>&& req,
                             std::string remote_host,
                             Send&& send) const {
    std::string_view target = req.target();
    target.remove_prefix(1);
    target_path_ = path_utils::url_decode(target);

    const std::vector<std::string_view> url_members =
        GetSplitUrlMembers(target_path_);

    if (!url_members.empty()) {
      if (url_members[0] == "api"s) {
        return WorkApi(url_members, req, send,
                       std::chrono::steady_clock::now());
      }

      return WorkStatic(req, send,
                        url_members.at(0).empty() ? "index.html" : target_path_,
                        std::chrono::steady_clock::now());
    }

    return WorkStatic(req, send, "index.html"s,
                      std::chrono::steady_clock::now());
  }

 private:
  template <typename Send>
  LogResponseData SendStringResponse(const StringRequest& req,
                                     const Send& send,
                                     std::string_view request_body,
                                     std::string_view content_type,
                                     http::status status,
                                     std::chrono::steady_clock::time_point start) const {
    const auto json_response = [&req, &content_type, this](
                                   http::status status, std::string_view text,
                                   bool is_set_body = true) {
      return MakeStringResponse(status, text, req, content_type, is_set_body);
    };

    auto resp =
        json_response(status, request_body, req.method() != http::verb::head);
    const auto finish = std::chrono::steady_clock::now();
    const auto diff = finish - start;
    const auto result_int = resp.result_int();
    send(std::move(resp));
    return {result_int,
            std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
            request_body.empty() ? "null" : content_type};
  }

  StringResponse MakeStringResponse(
      const http::status status,
      std::string_view body,
      const StringRequest& req,
      std::string_view content_type = ContentType::TEXT_HTML,
      bool is_set_body = true) const;

  template <typename Send>
  LogResponseData SendFileResponse(const StringRequest& req,
                                   const Send& send,
                                   file_body::value_type&& body,
                                   std::string_view content_type,
                                   http::status status,
                                   std::chrono::steady_clock::time_point start) const {
    auto resp = MakeFileResponse(status, std::move(body), content_type, req);
    const auto finish = std::chrono::steady_clock::now();
    const auto diff = finish - start;
    const auto result_int = resp.result_int();
    send(std::move(resp));
    return {result_int,
            std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
            content_type};
  }

  FileResponse MakeFileResponse(const http::status status,
                                file_body::value_type&& file,
                                std::string_view content_type,
                                const StringRequest& req) const;

  json::array GetArrayJsonMapHeads() const;

  json::object GetObjectJsonFromMap(const Map* const map) const;

  json::object GreateJsonMap(const Map* const map) const;

  void AddRoadsInJsonObject(json::object& json_map, const Map* const map) const;

  void AddBuildingsInJsonObject(json::object& json_map,
                                const Map* const map) const;

  void AddOfficesInJsonObject(json::object& json_map,
                              const Map* const map) const;

  template <typename Body, typename Allocator, typename Send>
  LogResponseData WorkApi(const std::vector<std::string_view>& url_members,
                          BasicRequest<Body, Allocator>& req,
                          Send& send,
                          std::chrono::steady_clock::time_point start) const {
    if (url_members[1] == "v1"s) {
      if (url_members[2] == "maps"s) {
        if (url_members.size() == 3) {
          return SendStringResponse(
              req, send, json::serialize(GetArrayJsonMapHeads()),
              ContentType::TEXT_JSON, http::status::ok, start);
        }

        if (url_members.size() == 4) {
          const auto id = Map::Id(std::string(url_members[3]));
          const auto map_ptr = game_.FindMap(id);
          if (!map_ptr) {
            return SendNotFoundResponse(
                req, send,
                json::parse(
                    R"({"code": "mapNotFound", "message": "Map not found"})"),
                start);
          }
          const auto json_map = GetObjectJsonFromMap(map_ptr);
          return SendStringResponse(req, send, json::serialize(json_map),
                                    ContentType::TEXT_JSON, http::status::ok,
                                    start);
        }
      }
    }

    return SendBadResponse(req, send, start);
  }

  template <typename Body, typename Allocator, typename Send>
  LogResponseData SendBadResponse(BasicRequest<Body, Allocator>& req,
                                  Send& send,
                                  std::chrono::steady_clock::time_point start) const {
    const static auto bad_request_body = json::parse(
        R"({"code": "badRequest", "message": "Bad request"})");

    return SendStringResponse(req, send, json::serialize(bad_request_body),
                              ContentType::TEXT_JSON, http::status::bad_request,
                              start);
  }

  template <typename Body, typename Allocator, typename Send>
  LogResponseData SendNotFoundResponse(BasicRequest<Body, Allocator>& req,
                                       Send& send,
                                       json::value json,
                                       std::chrono::steady_clock::time_point start) const {
    return SendStringResponse(req, send, json::serialize(json),
                              ContentType::TEXT_JSON, http::status::not_found,
                              start);
  }

  template <typename Body, typename Allocator, typename Send>
  LogResponseData SendNotFoundResponse(BasicRequest<Body, Allocator>& req,
                                       Send& send,
                                       std::string_view text,
                                       std::chrono::steady_clock::time_point start) const {
    return SendStringResponse(req, send, text, ContentType::TEXT_PLAIN,
                              http::status::not_found, start);
  }

  template <typename Body, typename Allocator, typename Send>
  LogResponseData WorkStatic(BasicRequest<Body, Allocator>& req,
                             Send& send,
                             std::string_view target,
                             std::chrono::steady_clock::time_point start) const {
    fs::path static_catalog =
        fs::weakly_canonical(fs::current_path() / "../" / static_catalog_);

    fs::path target_path{static_catalog / target};

    auto isSubCatalog = path_utils::IsSubPath(target_path, static_catalog);
    if (isSubCatalog) {
      const auto target_c_str = target_path.c_str();

      if (fs::is_regular_file(target_c_str)) {
        file_body::value_type file;
        if (sys::error_code ec;
            file.open(target_c_str, beast::file_mode::read, ec), ec) {
          // std::cerr << ec.message() << std::endl;
          return SendNotFoundResponse(req, send, "File not found"s, start);
        }
        std::string ext = target_path.extension().string();

        const auto iter = ContentType::Types.find(ext);

        if (iter == ContentType::Types.end()) {
          return SendFileResponse(req, send, std::move(file),
                                  "application/octet-stream", http::status::ok,
                                  start);
        }

        const auto& [_, mime_type] = *iter;

        return SendFileResponse(req, send, std::move(file), mime_type,
                                http::status::ok, start);
      }
      // return SendNotFoundResponse(req, send, ""s, start);
      // return SendStringResponse(req, send, ""s, "null"s, http::status::ok, start);
    }
    return SendNotFoundResponse(req, send, "File not found"s, start);
  }

 private:
  Game& game_;
  std::string static_catalog_;
  mutable std::string target_path_;
};

}  // namespace http_handler
