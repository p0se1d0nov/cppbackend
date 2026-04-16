#include "road.h"

#include <algorithm>

namespace model {

bool Road::IsHorizontal() const noexcept { return start_.y == end_.y; }
bool Road::IsVertical() const noexcept { return start_.x == end_.x; }

detail::Point Road::GetStart() const noexcept { return start_; }
detail::Point Road::GetEnd() const noexcept { return end_; }

detail::Point Road::GetJsonStart() const noexcept { return json_start_; }
detail::Point Road::GetJsonEnd() const noexcept { return json_end_; }

Road::Road(HorizontalTag, detail::Point start, detail::Coord end_x) noexcept
    : start_{start}, end_{end_x, start.y}, json_start_{start},
      json_end_{end_x, start.y} {
  if (start_.x > end_.x) {
    std::swap(start_.x, end_.x);
  }
}
Road::Road(VerticalTag, detail::Point start, detail::Coord end_y) noexcept
    : start_{start}, end_{start.x, end_y}, json_start_{start},
      json_end_{start.x, end_y} {
  if (start_.y > end_.y) {
    std::swap(start_.y, end_.y);
  }
}
} // namespace model