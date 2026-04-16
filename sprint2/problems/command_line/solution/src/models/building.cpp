#include "building.h"
namespace model {

const detail::Rectangle &Building::GetBounds() const noexcept {
  return bounds_;
}
} // namespace model