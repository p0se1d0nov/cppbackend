#pragma once

#include "details.h"

namespace model {
class Building {
public:
  explicit Building(detail::Rectangle bounds) noexcept : bounds_{bounds} {}

  const detail::Rectangle &GetBounds() const noexcept;

private:
  detail::Rectangle bounds_;
};
} // namespace model