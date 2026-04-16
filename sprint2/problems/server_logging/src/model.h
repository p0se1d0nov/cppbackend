#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "tagged.h"

namespace model {

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

  Road(HorizontalTag, Point start, Coord end_x) noexcept
      : start_{start}, end_{end_x, start.y} {}

  Road(VerticalTag, Point start, Coord end_y) noexcept
      : start_{start}, end_{start.x, end_y} {}

  bool IsHorizontal() const noexcept;

  bool IsVertical() const noexcept;

  Point GetStart() const noexcept;

  Point GetEnd() const noexcept;

private:
  Point start_;
  Point end_;
};

class Building {
public:
  explicit Building(Rectangle bounds) noexcept : bounds_{bounds} {}

  const Rectangle &GetBounds() const noexcept;

private:
  Rectangle bounds_;
};

class Office {
public:
  using Id = util::Tagged<std::string, Office>;

  Office(Id id, Point position, Offset offset) noexcept
      : id_{std::move(id)}, position_{position}, offset_{offset} {}

  const Id &GetId() const noexcept;

  Point GetPosition() const noexcept;

  Offset GetOffset() const noexcept;

private:
  Id id_;
  Point position_;
  Offset offset_;
};

class Map {
public:
  using Id = util::Tagged<std::string, Map>;
  using Roads = std::vector<Road>;
  using Buildings = std::vector<Building>;
  using Offices = std::vector<Office>;

  Map(Id id, std::string name) noexcept
      : id_(std::move(id)), name_(std::move(name)) {}

  const Id &GetId() const noexcept;

  const std::string &GetName() const noexcept;

  const Buildings &GetBuildings() const noexcept;

  const Roads &GetRoads() const noexcept;

  const Offices &GetOffices() const noexcept;

  void AddRoad(const Road &road);

  void AddBuilding(const Building &building);

  void AddOffice(Office office);

private:
  using OfficeIdToIndex =
      std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

  Id id_;
  std::string name_;
  Roads roads_;
  Buildings buildings_;

  OfficeIdToIndex warehouse_id_to_index_;
  Offices offices_;
};

class Game {
public:
  using Maps = std::vector<Map>;

  void AddMap(Map map);

  const Maps &GetMaps() const noexcept;

  const Map *FindMap(const Map::Id &id) const noexcept;

private:
  using MapIdHasher = util::TaggedHasher<Map::Id>;
  using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

  std::vector<Map> maps_;
  MapIdToIndex map_id_to_index_;
};

} // namespace model
