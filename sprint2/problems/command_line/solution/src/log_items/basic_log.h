#pragma once

#include <boost/json.hpp>
#include <string_view>

namespace server_log {
namespace sys = boost::system;
using namespace std::literals;
class IBasicLog {
public:
  IBasicLog() = default;
  IBasicLog(const IBasicLog &) = delete;
  IBasicLog(IBasicLog &&) = delete;
  IBasicLog &operator=(const IBasicLog &) = delete;
  IBasicLog &operator=(IBasicLog &&) = delete;

  virtual ~IBasicLog() = default;

  virtual void OnStart(unsigned int port, std::string_view address) const = 0;
  virtual void OnExit(const sys::error_code &ec) const = 0;
  virtual void OnExit(const std::exception &ex) const = 0;
  virtual void OnError(const sys::error_code &ec,
                       std::string_view where) const = 0;

  virtual void OnRequest(std::string_view remote_host, std::string_view target,
                         std::string_view method) const = 0;

  virtual void OnResponse(long int response_time, unsigned int code,
                          std::string_view content_type) const = 0;

  virtual const IBasicLog &GetInstance() const = 0;
};

} // namespace server_log