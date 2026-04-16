#pragma once

#include "api_handler.h"
#include "boost_beast.h"
#include "game.h"
#include "log_response_data.h"
#include "request_constant_values.h"
#include "request_handler_utils.h"
#include <boost/algorithm/string.hpp>

namespace http_handler {

using namespace std::literals;
using namespace model;

class ApiWorker {
public:
  ApiWorker(Game &game) : game_{game}, api_handler_{game} {};

  template <typename Body, typename Allocator, typename Send>
  LogResponseData
  operator()(const std::vector<std::string> &url_members,
             request<Body, basic_fields<Allocator>> &req, Send &send,
             const std::chrono::steady_clock::time_point &start) const {
    const auto method = PrepareSendMethod(req, send, start, true);
    if (url_members.size() > 2 && url_members.at(0) == "api"s) {
      return api_handler_(url_members.begin() + 1, url_members.end(), method,
                          req);
    }

    return method(http::status::bad_request, Routes::BAD_REQUEST);
  }

private:
  Game &game_;
  Routes::ApiHandler api_handler_;
};
} // namespace http_handler