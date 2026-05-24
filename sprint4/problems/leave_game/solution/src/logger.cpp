#include "logger.h"
#include <boost/log/support/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>  // для add_value

namespace logging {

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

void JsonFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    json::object obj;

    // Исправляем: "TimeStamp" как ключ, а не logging::attribute_value
    auto ts = rec["TimeStamp"].extract<boost::posix_time::ptime>();
    if (ts) {
        obj["timestamp"] = boost::posix_time::to_iso_extended_string(ts.get());
    }

    auto msg = rec[expr::smessage];
    if (msg) {
        obj["message"] = msg.get();
    }

    auto data = rec[additional_data];
    if (data) {
        obj["data"] = data.get();
    } else {
        obj["data"] = json::object{};
    }

    strm << json::serialize(obj);
}

void InitBoostLog() {
    logging::add_common_attributes();

    auto sink = logging::add_console_log(
        std::cout,
        keywords::format = &JsonFormatter,
        keywords::auto_flush = true
        );

    sink->set_filter(logging::trivial::severity >= logging::trivial::info);
}

void LogServerStarted(unsigned short port, const std::string& address) {
    json::object data;
    data["port"] = port;
    data["address"] = address;
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, data) << "server started";
}

void LogServerExited(int code, const std::string& exception_msg) {
    json::object data;
    data["code"] = code;
    if (!exception_msg.empty()) {
        data["exception"] = exception_msg;
    }
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, data) << "server exited";
}

void LogRequest(const std::string& ip, const std::string& uri, const std::string& method) {
    json::object data;
    data["ip"] = ip;
    data["URI"] = uri;
    data["method"] = method;
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, data) << "request received";
}

void LogResponse(const std::chrono::steady_clock::time_point& start_time, int status_code, const std::string& content_type) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    json::object data;
    data["response_time"] = elapsed_ms;
    data["code"] = status_code;
    if (!content_type.empty()) {
        data["content_type"] = content_type;
    } else {
        data["content_type"] = nullptr;
    }
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, data) << "response sent";
}

void LogError(int code, const std::string& text, const std::string& where) {
    json::object data;
    data["code"] = code;
    data["text"] = text;
    data["where"] = where;
    BOOST_LOG_TRIVIAL(error) << boost::log::add_value(additional_data, data) << "error";
}

} // namespace logging
