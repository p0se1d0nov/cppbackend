#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
  auto GetTime() const;

  std::string GetTimeStamp() const;

  // Для имени файла возьмите дату с форматом "%Y_%m_%d"
  std::string GetFileTimeStamp() const;

  Logger() = default;
  Logger(const Logger &) = delete;

public:
  static Logger &GetInstance() {
    static Logger obj;
    return obj;
  }

  template <class... Ts> void Log(const Ts &...args) {
    std::lock_guard lock(mut_);
    // SetTimestamp(std::chrono::system_clock::now());
    const auto file_path = "/var/log/sample_log_" + GetFileTimeStamp() + ".log";
    if (!log_file_) {
      OpenFile(file_path);
    }

    if (!log_file_) {
      std::cerr << "Exception in open file: " << file_path << std::endl;
      return;
    }

    if (file_path_ != file_path) {
      log_file_.close();
      OpenFile(file_path);
    }

    if (!log_file_) {
      std::cerr << "Exception in open file: " << file_path << std::endl;
      return;
    }
    log_file_ << GetTimeStamp() << ": ";

    ((log_file_ << args), ...);
    log_file_ << std::endl;
  };

  // Установите manual_ts_. Учтите, что эта операция может выполняться
  // параллельно с выводом в поток, вам нужно предусмотреть
  // синхронизацию.
  void SetTimestamp(std::chrono::system_clock::time_point ts);

private:
  void OpenFile(const std::string &file_path) {
    log_file_.open(file_path, std::ios::app);
    file_path_ = file_path;
  }

private:
  std::optional<std::chrono::system_clock::time_point> manual_ts_;
  std::mutex mut_;
  std::ofstream log_file_;
  std::string file_path_{};
};
