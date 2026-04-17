#include "boost_log_shell.h"
#include "log_attributes.h"

namespace server_log {

namespace logging = boost::log;
namespace json = boost::json;
namespace sys = boost::system;
namespace keywords = boost::log::keywords;

void MyFormatter(logging::record_view const &rec,
                 logging::formatting_ostream &strm) {

  strm << "{\"timestamp\":\""
       << to_iso_extended_string(
              boost::posix_time::microsec_clock::local_time())
       << "\",\"data\":"s
       << logging::extract<json::value>("AdditionalData", rec)
       << ",\"message\":\"" << rec[logging::expressions::smessage] << "\"}";
};

BoostLogShell::BoostLogShell() {
  logging::add_console_log(std::clog, keywords::format = &MyFormatter,
                           keywords::auto_flush = true);
}
void BoostLogShell::OnStart(unsigned int port, std::string_view address) const {
  BOOST_LOG_TRIVIAL(info)
      << logging::add_value(additional_data,
                            json::value{{"port"s, port}, {"address"s, address}})
      << "server started"sv;
}

void BoostLogShell::OnExit(const sys::error_code &ec) const {
  if (!ec) {
    BOOST_LOG_TRIVIAL(info)
        << logging::add_value(additional_data,
                              json::value{{"code"s, ec.value()}})
        << "server exited";
  } else {
    BOOST_LOG_TRIVIAL(error)
        << logging::add_value(
               additional_data,
               json::value{{"code"s, ec.value()}, {"exception", ec.what()}})
        << "server exited";
  }
}

void BoostLogShell::OnExit(const std::exception &ex) const {
  BOOST_LOG_TRIVIAL(error) << logging::add_value(
                                  additional_data,
                                  json::value{{"code"s, EXIT_FAILURE},
                                              {"exception", ex.what()}})
                           << "server exited";
}

void BoostLogShell::OnError(const sys::error_code &ec,
                            std::string_view where) const {
  BOOST_LOG_TRIVIAL(error) << logging::add_value(
                                  additional_data,
                                  json::value{{"code"s, ec.value()},
                                              {"text", ec.message()},
                                              {"where", where}})
                           << "error";
}

void BoostLogShell::OnRequest(std::string_view remote_host,
                              std::string_view target,
                              std::string_view method) const {
  BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                 additional_data,
                                 json::value{{"ip"s, remote_host},
                                             {"URI"s, target},
                                             {"method"s, method}})
                          << "request received"sv;
};

void BoostLogShell::OnResponse(long int response_time, unsigned int code,
                               std::string_view content_type) const {
  BOOST_LOG_TRIVIAL(info) << logging::add_value(
                                 additional_data,
                                 json::value{{"response_time"s, response_time},
                                             {"code"s, code},
                                             {"content_type"s, content_type}})
                          << "response sent"sv;
};

const BoostLogShell &BoostLogShell::GetInstance() const {
  const static BoostLogShell instance{};
  return instance;
}

const IBasicLog &GetLogShell() {
  const static BoostLogShell log_shell{};
  return log_shell;
}

} // namespace server_log