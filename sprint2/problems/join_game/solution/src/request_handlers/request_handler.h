#pragma once
#include <cassert>
#include <string_view>
#include <utility>
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "api_worker.h"
#include "game.h"
#include "http_server.h"
#include "log_response_data.h"
#include "path_utils.h"
#include "request_handler_utils.h"

#include <boost/algorithm/string.hpp>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;

using namespace std::literals;
using namespace model;
template <typename Body, typename Allocator>
using BasicRequest = request<Body, basic_fields<Allocator>>;

using FileResponse = response<file_body>;

class RequestHandler {
public:
  explicit RequestHandler(Game &game, std::string static_catalog);

  RequestHandler(const RequestHandler &) = delete;
  RequestHandler &operator=(const RequestHandler &) = delete;

  RequestHandler(RequestHandler &&handler) noexcept;

  RequestHandler &operator=(RequestHandler &&handler) noexcept;

  template <typename Body, typename Allocator, typename Send>
  LogResponseData operator()(BasicRequest<Body, Allocator> &&req,
                             std::string remote_host, Send &&send) const {
    const auto start = std::chrono::steady_clock::now();
    std::string_view target = req.target();
    target.remove_prefix(1);
    const auto target_path_ = util::url_decode(target);

    const std::vector<std::string> url_members =
        GetSplitUrlMembers(target_path_);

    if (!url_members.empty()) {
      if (url_members[0] == "api"s) {
        return WorkApi(url_members, req, send, start);
      }

      return WorkStatic(req, send,
                        url_members.at(0).empty() ? "index.html" : target_path_,
                        start);
    }

    return WorkStatic(req, send, "index.html"s, start);
  }

private:
  template <typename Body, typename Allocator, typename Send>
  LogResponseData
  WorkApi(const std::vector<std::string> &url_members,
          BasicRequest<Body, Allocator> &req, Send &send,
          const std::chrono::steady_clock::time_point &start) const {

    return api_worker_(url_members, req, send, start);
  }

  template <typename Body, typename Allocator, typename Send>
  LogResponseData
  WorkStatic(BasicRequest<Body, Allocator> &req, Send &send,
             std::string_view target,
             const std::chrono::steady_clock::time_point &start) const {
    std::filesystem::path static_catalog = std::filesystem::weakly_canonical(
        std::filesystem::current_path() / "../" / static_catalog_);

    std::filesystem::path target_path{static_catalog / target};

    auto isSubCatalog = util::IsSubPath(target_path, static_catalog);
    if (isSubCatalog) {
      const auto target_c_str = target_path.c_str();

      if (std::filesystem::is_regular_file(target_c_str)) {
        file_body::value_type file;
        if (boost::system::error_code ec;
            file.open(target_c_str, beast::file_mode::read, ec), ec) {
          return SendNotFoundResponse(req, send, "File not found"s, start);
        }
        std::string ext = target_path.extension().string();

        const auto iter = ContentType::Types.find(ext);

        if (iter == ContentType::Types.end()) {
          return SendFileResponse(req, send, std::move(file),
                                  "application/octet-stream", http::status::ok,
                                  start);
        }

        const auto &[_, mime_type] = *iter;

        return SendFileResponse(req, send, std::move(file), mime_type,
                                http::status::ok, start);
      }
    }
    return SendNotFoundResponse(req, send, "File not found"s, start);
  }

private:
  Game &game_;
  http_handler::ApiWorker api_worker_;
  std::string static_catalog_;
};

} // namespace http_handler
