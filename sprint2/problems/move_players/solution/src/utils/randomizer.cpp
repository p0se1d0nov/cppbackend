#include "randomizer.h"

#include <iomanip>
#include <sstream>

std::string util::Randomizer::operator()() {
  std::mt19937_64 generator1_{[this] { return dist_(random_device_); }()};

  std::mt19937_64 generator2_{[this] { return dist_(random_device_); }()};

  std::stringstream stream;
  stream << std::hex << std::setw(16) << std::setfill('0') << generator1_()
         << std::setw(16) << std::setfill('0') << generator2_();
  return stream.str();
}
