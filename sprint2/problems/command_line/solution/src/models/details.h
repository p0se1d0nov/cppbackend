#pragma once

namespace model {
namespace detail {

const inline static auto ROAD_OFFSET = 0.4;

using Dimension = int;
using Coord = Dimension;

struct Point {
  Coord x, y;
};

struct Size {
  Dimension width, height;
};

struct Rectangle {
  Point position;
  Size size;
};

struct Offset {
  Dimension dx, dy;
};

using DogCoord = double;
using Speed = double;

struct Coordinate {
  DogCoord x = 0;
  DogCoord y = 0;
  explicit Coordinate(const Point &p) : x(p.x), y(p.y) {}
};

struct DirectionSpeed {
  Speed x = 0;
  Speed y = 0;
};

enum class Direction { STOP, NORTH, EAST, SOUTH, WEST };

} // namespace detail

} // namespace model
