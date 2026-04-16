#include "my_logger.h"

auto Logger::GetTime() const {
  if (manual_ts_) {
    return *manual_ts_;
  }

  return std::chrono::system_clock::now();
}

std::string Logger::GetTimeStamp() const {
  //   const auto now =
  //       std::chrono::time_point_cast<std::chrono::seconds>(GetTime());
  //   return std::format("{:%F %T}", now);

  const auto now = GetTime();
  const auto t_c = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&t_c), "%F %T");
  return ss.str();
}

std::string Logger::GetFileTimeStamp() const {
  const auto now = GetTime();

  const auto t_c = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&t_c), "%Y_%m_%d");
  return ss.str();
}

void Logger::SetTimestamp(std::chrono::system_clock::time_point ts) {
  manual_ts_ = ts;
}
