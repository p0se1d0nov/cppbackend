#pragma once

#include "boost_beast.h"
#include "boost_json.h"
#include "log_response_data.h"

namespace Routes {

namespace http = boost::beast::http;

using It = std::vector<std::string>::const_iterator;
using Method = std::function<http_handler::LogResponseData(
    http::status, boost::json::value, [[maybe_unused]] std::string_view)>;
using Request = http::request<http::string_body>;

} // namespace Routes