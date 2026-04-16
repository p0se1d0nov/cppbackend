
#include "request_handler.h"

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include <string_view>

namespace http_handler {

StringResponse MakeStringResponse(const http::status status,
                                  std::string_view body,
                                  const StringRequest &req,
                                  std::string_view content_type,
                                  bool is_set_body, bool is_no_cache,
                                  std::string_view allow_method) {
  StringResponse res(status, req.version());
  res.set(http::field::content_type, content_type);
  if (is_set_body) {
    res.body() = body;
  }
  if (!allow_method.empty()) {
    res.set(http::field::allow, allow_method);
  }
  res.content_length(body.size());
  res.keep_alive(req.keep_alive());
  if (is_no_cache) {
    res.set(http::field::cache_control, "no-cache");
  }

  return res;
}

FileResponse MakeFileResponse(const http::status status,
                              file_body::value_type &&file,
                              std::string_view content_type,
                              const StringRequest &req) {
  response<file_body> res;
  res.version(11); // HTTP/1.1
  res.result(status::ok);
  res.insert(field::content_type, content_type);

  res.body() = std::move(file);
  // Метод prepare_payload заполняет заголовки Content-Length и
  // Transfer-Encoding в зависимости от свойств тела сообщения
  res.prepare_payload();
  return res;
};

std::vector<std::string> GetSplitUrlMembers(const std::string &view) {
  std::vector<std::string> url_members;

  boost::split(url_members, view, boost::is_any_of("/"));
  return url_members;
}

} // namespace http_handler