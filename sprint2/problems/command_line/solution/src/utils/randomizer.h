#pragma once

#include <random>

namespace util {
class Randomizer {
public:
  Randomizer() = default;

private:
  std::random_device random_device_;
  std::uniform_int_distribution<std::mt19937_64::result_type> dist_;

public:
  std::string operator()();
};
} // namespace util