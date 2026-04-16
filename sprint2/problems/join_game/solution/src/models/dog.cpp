#include "dog.h"
#include "details.h"

#include <chrono>
#include <string>
namespace model {

constexpr double MILLISECONDS_PER_SECOND = 1000.0;

const std::string &Dog::GetId() const { return dog_id_; }

detail::Coordinate Dog::GetPosition() const { return coordinate_; };

detail::Speed Dog::GetSpeed() const { return speed_; };

detail::DirectionSpeed Dog::GetDirectionSpeed() const {
  switch (direction_) {
  case detail::Direction::NORTH:
    return {0, -speed_};
  case detail::Direction::EAST:
    return {speed_, 0};
  case detail::Direction::SOUTH:
    return {0, speed_};
  case detail::Direction::WEST:
    return {-speed_, 0};
  default:
    return {0, 0};
  }
};

void Dog::Move(std::chrono::steady_clock::duration duration) {
  if (direction_ == detail::Direction::STOP) {
    return;
  }
  double time =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() /
      MILLISECONDS_PER_SECOND;

  const auto [vertical, horizontal] =
      map_ptr_->GetRoadsByCoordinate(coordinate_, direction_);

  switch (direction_) {
  case detail::Direction::NORTH: {
    if (vertical) {
      const auto border = vertical->GetStart().y - detail::ROAD_OFFSET;
      coordinate_.y = std::max(border, coordinate_.y - speed_ * time);
      if (coordinate_.y <= border) {
        speed_ = 0;
      }
      break;
    }
    if (horizontal) {
      const auto border = horizontal->GetStart().y - detail::ROAD_OFFSET;
      coordinate_.y = std::max(border, coordinate_.y - speed_ * time);
      if (coordinate_.y <= border) {
        speed_ = 0;
      }
      break;
    }
    break;
  }
  case detail::Direction::EAST: {
    if (horizontal) {
      const auto border = horizontal->GetEnd().x + detail::ROAD_OFFSET;
      coordinate_.x = std::min(border, coordinate_.x + speed_ * time);
      if (coordinate_.x >= border) {
        speed_ = 0;
      }
      break;
    }
    if (vertical) {
      const auto border = vertical->GetEnd().x + detail::ROAD_OFFSET;
      coordinate_.x = std::min(border, coordinate_.x + speed_ * time);
      if (coordinate_.x >= border) {
        speed_ = 0;
      }
      break;
    }
    break;
  }
  case detail::Direction::SOUTH: {
    if (vertical) {
      const auto border = vertical->GetEnd().y + detail::ROAD_OFFSET;

      coordinate_.y = std::min(border, coordinate_.y + speed_ * time);
      if (coordinate_.y >= border) {
        speed_ = 0;
      }
      break;
    }
    if (horizontal) {
      const auto border = horizontal->GetEnd().y + detail::ROAD_OFFSET;

      coordinate_.y = std::min(border, coordinate_.y + speed_ * time);
      if (coordinate_.y >= border) {
        speed_ = 0;
      }
      break;
    }
    break;
  }
  case detail::Direction::WEST: {
    if (horizontal) {
      const auto border = horizontal->GetStart().x - detail::ROAD_OFFSET;

      coordinate_.x = std::max(border, coordinate_.x - speed_ * time);
      if (coordinate_.x <= border) {
        speed_ = 0;
      }
      break;
    }
    if (vertical) {
      const auto border = vertical->GetStart().x - detail::ROAD_OFFSET;

      coordinate_.x = std::max(border, coordinate_.x - speed_ * time);
      if (coordinate_.x <= border) {
        speed_ = 0;
      }
      break;
    }
    break;
  }
  default:
    break;
  }
}

detail::Direction Dog::GetDirection() const { return direction_; };

Dog::State Dog::GetState() const {
  return {coordinate_, GetSpeed(), direction_, GetDirectionSpeed()};
}

void Dog::SetSpeed(detail::Speed speed) { speed_ = speed; };
void Dog::SetDirection(detail::Direction direction) {
  direction_ = direction;
  speed_ = base_speed_;
};

} // namespace model