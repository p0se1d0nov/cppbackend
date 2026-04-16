#pragma once
#include "details.h"
#include "map.h"

#include <chrono>
#include <string>

namespace model {

class Dog {
public:
  struct State {
    detail::Coordinate coordinate;
    detail::Speed speed;
    detail::Direction direction;
    detail::DirectionSpeed direction_speed;
  };

public:
  Dog(std::string dog_id, const model::Map *map_ptr)
      : dog_id_(std::move(dog_id)), map_ptr_{map_ptr}, speed_{0},
        base_speed_{map_ptr->GetDogSpeed()},
        coordinate_{map_ptr->GetDefaultDogPosition()} {}

public:
  const std::string &GetId() const;

  detail::Coordinate GetPosition() const;
  detail::Speed GetSpeed() const;
  detail::DirectionSpeed GetDirectionSpeed() const;
  detail::Direction GetDirection() const;

  void Move(std::chrono::steady_clock::duration duration);

  State GetState() const;

  void SetSpeed(detail::Speed speed);
  void SetDirection(detail::Direction direction);

private:
  std::string dog_id_;
  detail::Speed speed_;
  const detail::Speed base_speed_;
  const model::Map *map_ptr_;
  detail::Coordinate coordinate_;
  detail::Direction direction_ = detail::Direction::NORTH;
};
} // namespace model
