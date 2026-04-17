#include "office.h"

namespace model {

const Office::Id &Office::GetId() const noexcept { return id_; }
detail::Point Office::GetPosition() const noexcept { return position_; }
detail::Offset Office::GetOffset() const noexcept { return offset_; }
} // namespace model