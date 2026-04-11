#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger
{
    mutable std::mutex mutex_;
    std::ofstream file_;
    std::string current_date_;

    auto GetTime() const
    {
        std::lock_guard lock(mutex_);
        if (manual_ts_)
        {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    std::string FormatTime(std::chrono::system_clock::time_point tp, const char *fmt) const
    {
        const auto t_c = std::chrono::system_clock::to_time_t(tp);
        std::tm tm;
#if defined(_MSC_VER)
        localtime_s(&tm, &t_c);
#else
        localtime_r(&t_c, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, fmt);
        return oss.str();
    }

    std::string GetTimeStamp() const
    {
        const auto now = GetTime();
        return FormatTime(now, "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const
    {
        const auto now = GetTime();
        return FormatTime(now, "%Y_%m_%d");
    }

    void EnsureFileOpen(const std::string &date)
    {
        if (date != current_date_ || !file_.is_open())
        {
            current_date_ = date;
            if (file_.is_open())
            {
                file_.close();
            }

            std::string filename = "/var/log/sample_log_" + date + ".log";
            file_.open(filename, std::ios::app);
        }
    }

    Logger() = default;
    Logger(const Logger &) = delete;

public:
    static Logger &GetInstance()
    {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template <class... Ts>
    void Log(const Ts &...args)
    {
        std::lock_guard lock(mutex_);

        auto now = manual_ts_.value_or(std::chrono::system_clock::now());
        std::string timestamp = FormatTime(now, "%F %T");
        std::string file_date = FormatTime(now, "%Y_%m_%d");

        EnsureFileOpen(file_date);

        if (!file_.is_open())
        {
            return;
        }

        file_ << timestamp << ": ";
        (file_ << ... << args);
        file_ << '\n';
        file_.flush();
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts)
    {
        std::lock_guard lock(mutex_);
        manual_ts_ = ts;
    }

private:
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
};
