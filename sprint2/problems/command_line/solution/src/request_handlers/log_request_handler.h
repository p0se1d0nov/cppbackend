#pragma once
#include "boost_log_shell.h"
#include "request_handler.h"

#include <string>
#include <utility>

namespace http_handler {

template <class SomeRequestHandler> class LogRequestHandler {
public:
  LogRequestHandler(SomeRequestHandler handler)
      : handler_(std::move(handler)) {}

  LogRequestHandler(const LogRequestHandler &) = delete;
  LogRequestHandler &operator=(const LogRequestHandler &) = delete;

  template <typename Body, typename Allocator, typename Send>
  void operator()(request<Body, basic_fields<Allocator>> &&req,
                  std::string remote_host, Send &&send) {

    server_log::GetLogShell().OnRequest(remote_host, req.target(),
                                        req.method_string());

    const auto resp =
        handler_(std::move(req), std::move(remote_host), std::move(send));

    server_log::GetLogShell().OnResponse(resp.response_time, resp.code,
                                         resp.content_type);
  }

private:
  const SomeRequestHandler handler_;
};

} // namespace http_handler
