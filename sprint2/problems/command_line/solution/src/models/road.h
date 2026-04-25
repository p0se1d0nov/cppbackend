#pragma once
#include "details.h"

namespace model {
class Road {
  struct HorizontalTag {
    explicit HorizontalTag() = default;
  };

  struct VerticalTag {
    explicit VerticalTag() = default;
  };

public:
  constexpr static HorizontalTag HORIZONTAL{};
  constexpr static VerticalTag VERTICAL{};

  Road(HorizontalTag, detail::Point start, detail::Coord end_x) noexcept;

  Road(VerticalTag, detail::Point start, detail::Coord end_y) noexcept;

  bool IsHorizontal() const noexcept;

  bool IsVertical() const noexcept;

  detail::Point GetStart() const noexcept;
  detail::Point GetEnd() const noexcept;

  detail::Point GetJsonStart() const noexcept;
  detail::Point GetJsonEnd() const noexcept;

private:
  detail::Point start_;
  detail::Point end_;
  detail::Point json_start_;
  detail::Point json_end_;
};
} // namespace model