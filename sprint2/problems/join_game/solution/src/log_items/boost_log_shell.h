#pragma once

#include "basic_log.h"

#include <boost/date_time.hpp>
#include <string_view>

namespace server_log {

namespace sys = boost::system;

class BoostLogShell : public IBasicLog {
public:
  BoostLogShell();
  void OnStart(unsigned int port, std::string_view address) const override;

  void OnExit(const sys::error_code &ec) const override;
  void OnExit(const std::exception &ex) const override;

  void OnError(const sys::error_code &ec,
               std::string_view where) const override;

  void OnRequest(std::string_view remote_host, std::string_view target,
                 std::string_view method) const override;

  void OnResponse(long int response_time, unsigned int code,
                  std::string_view content_type) const override;

  const BoostLogShell &GetInstance() const override;
};

const IBasicLog &GetLogShell();

} // namespace server_log