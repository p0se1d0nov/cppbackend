#pragma once
#include "boost_beast.h"
#include "boost_json.h"
#include "content_type.h"
#include "log_response_data.h"

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <string_view>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

using namespace http;
using namespace std::literals;
template <typename Body, typename Allocator>
using BasicRequest = request<Body, basic_fields<Allocator>>;

using StringRequest = request<string_body>;
using StringResponse = response<string_body>;
using FileResponse = response<file_body>;

StringResponse MakeStringResponse(const http::status status,
                                  std::string_view body,
                                  const StringRequest &req,
                                  std::string_view content_type,
                                  bool is_set_body, bool is_no_cache,
                                  std::string_view allow_method);

FileResponse MakeFileResponse(const http::status status,
                              file_body::value_type &&file,
                              std::string_view content_type,
                              const StringRequest &req);

template <typename Send>
inline LogResponseData SendStringResponse(
    const StringRequest &req, const Send &send, std::string_view request_body,
    std::string_view content_type, http::status status,
    const std::chrono::steady_clock::time_point &start,
    bool is_no_cache = false, std::string_view allow_method = ""s) {
  const auto json_response = [&req, &content_type, allow_method](
                                 http::status status, std::string_view text,
                                 bool is_set_body = true,
                                 bool is_no_cache = false) {
    return MakeStringResponse(status, text, req, content_type, is_set_body,
                              is_no_cache, allow_method);
  };

  auto resp = json_response(status, request_body,
                            req.method() != http::verb::head, is_no_cache);
  const auto finish = std::chrono::steady_clock::now();
  const auto diff = finish - start;
  const auto result_int = resp.result_int();
  send(std::move(resp));
  return {result_int,
          std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
          request_body.empty() ? "null" : content_type};
}

template <typename Send>
inline LogResponseData
SendFileResponse(const StringRequest &req, const Send &send,
                 file_body::value_type &&body, std::string_view content_type,
                 http::status status,
                 const std::chrono::steady_clock::time_point &start) {
  auto resp = MakeFileResponse(status, std::move(body), content_type, req);
  const auto finish = std::chrono::steady_clock::now();
  const auto diff = finish - start;
  const auto result_int = resp.result_int();
  send(std::move(resp));
  return {result_int,
          std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
          content_type};
}

template <typename Body, typename Allocator, typename Send>
inline auto
PrepareSendMethod(BasicRequest<Body, Allocator> &req, Send &send,
                  const std::chrono::steady_clock::time_point &start,
                  bool is_no_cache) {
  return [&req, &send, &start,
          is_no_cache](http::status status, json::value json,
                       [[maybe_unused]] std::string_view allow_method = "") {
    return SendStringResponse(req, send, json::serialize(json),
                              ContentType::TEXT_JSON, status, start,
                              is_no_cache, allow_method);
  };
}

template <typename Body, typename Allocator, typename Send>
inline LogResponseData
SendNotFoundResponse(BasicRequest<Body, Allocator> &req, Send &send,
                     std::string_view text,
                     const std::chrono::steady_clock::time_point &start,
                     bool is_no_cache = false) {
  return SendStringResponse(req, send, text, ContentType::TEXT_PLAIN,
                            http::status::not_found, start, is_no_cache);
}

std::vector<std::string> GetSplitUrlMembers(const std::string &view);

} // namespace http_handler