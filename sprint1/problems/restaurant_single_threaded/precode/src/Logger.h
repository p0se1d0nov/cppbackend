#pragma once

#include <syncstream>
#include <iostream>
#include <chrono>

using namespace std::chrono;
using namespace std::literals;

class Logger {
public:
    explicit Logger(std::string id);
    void LogMessage(std::string_view message) const;

private:
    std::string id_;
    steady_clock::time_point start_time_{steady_clock::now()};
};
