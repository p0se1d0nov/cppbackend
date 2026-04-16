#pragma once

#include "details.h"
#include "tagged.h"

#include <string>

namespace model {
class Office {
public:
  using Id = util::Tagged<std::string, Office>;

  Office(Id id, detail::Point position, detail::Offset offset) noexcept
      : id_{std::move(id)}, position_{position}, offset_{offset} {}

  const Id &GetId() const noexcept;

  detail::Point GetPosition() const noexcept;

  detail::Offset GetOffset() const noexcept;

private:
  Id id_;
  detail::Point position_;
  detail::Offset offset_;
};
} // namespace model