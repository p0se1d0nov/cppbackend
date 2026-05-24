#pragma once
#include <random>
#include <string>
#include <iomanip>
#include <sstream>

class TokenGenerator {
public:
    TokenGenerator() {
        std::random_device rd;
        gen1_.seed(rd());
        gen2_.seed(rd());
    }

    std::string Generate() {
        std::uint64_t high = dist_(gen1_);
        std::uint64_t low = dist_(gen2_);
        std::ostringstream oss;
        oss << std::hex << std::setfill('0')
            << std::setw(16) << high
            << std::setw(16) << low;
        return oss.str();
    }

private:
    std::mt19937_64 gen1_;
    std::mt19937_64 gen2_;
    std::uniform_int_distribution<std::uint64_t> dist_;
};
