#include "Logger.h"

#include <string>

Logger::Logger(std::string id)
    : id_(std::move(id)) {
}

void Logger::LogMessage(std::string_view message) const {
    std::osyncstream os{std::cout};
    os << id_ << "> ["sv << duration<double>(steady_clock::now() - start_time_).count()
        << "s] "sv << message << std::endl;
}