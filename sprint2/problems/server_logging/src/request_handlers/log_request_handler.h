#pragma once
#include "log_attributes.h"
#include "request_handler.h"
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace http_handler {

namespace logging = boost::log;

template <class SomeRequestHandler> class LogRequestHandler {
public:
  LogRequestHandler(SomeRequestHandler handler)
      : handler_(std::move(handler)) {}

  LogRequestHandler(const LogRequestHandler &) = delete;
  LogRequestHandler &operator=(const LogRequestHandler &) = delete;

  template <typename Body, typename Allocator, typename Send>
  void operator()(BasicRequest<Body, Allocator> &&req, std::string remote_host,
                  Send &&send) {

    BOOST_LOG_TRIVIAL(info)
        << logging::add_value(additional_data,
                              json::value{{"ip"s, remote_host},
                                          {"URI"s, req.target()},
                                          {"method"s, req.method_string()}})
        << "request received"sv;

    const auto resp =
        handler_(std::move(req), std::move(remote_host), std::move(send));

    BOOST_LOG_TRIVIAL(info)
        << logging::add_value(
               additional_data,
               json::value{{"response_time"s, resp.response_time},
                           {"code"s, resp.code},
                           {"content_type"s, resp.content_type}})
        << "response sent"sv;
  }

private:
  const SomeRequestHandler handler_;
};

} // namespace http_handler
