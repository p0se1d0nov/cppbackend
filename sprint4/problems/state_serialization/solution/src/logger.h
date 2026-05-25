#pragma once

// Сначала Boost.Log
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

// Затем Boost.JSON
#include <boost/json.hpp>

#include <chrono>
#include <string>

namespace logging {

namespace json = boost::json;

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)

void InitBoostLog();

void LogServerStarted(unsigned short port, const std::string& address);
void LogServerExited(int code, const std::string& exception_msg = "");
void LogRequest(const std::string& ip, const std::string& uri, const std::string& method);
void LogResponse(const std::chrono::steady_clock::time_point& start_time, int status_code, const std::string& content_type);
void LogError(int code, const std::string& text, const std::string& where);

} // namespace logging
