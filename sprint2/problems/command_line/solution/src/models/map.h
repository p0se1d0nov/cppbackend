#pragma once

#include "details.h"
#include "tagged.h"

#include "building.h"
#include "office.h"
#include "road.h"

#include <deque>
#include <map>
#include <unordered_map>
#include <utility>

namespace model {

class Map {
public:
  using Id = util::Tagged<std::string, Map>;
  using Roads = std::deque<Road>;
  using Buildings = std::vector<Building>;
  using Offices = std::vector<Office>;
  using RoadsMap = std::map<detail::Coord, const Road *>;
  using RoadsBase = std::unordered_map<detail::Coord, RoadsMap>;
  using RoadsBaseIterator = RoadsBase::const_iterator;
  using GetCoordsMethod =
      std::pair<detail::Coord, detail::Coord> (Map::*)(const Road *) const;

  struct PairRoads {
    const Road *vertical;
    const Road *horizontal;
  };

  Map(Id id, std::string name, detail::Speed dog_speed,
      bool randomize_spawn_point) noexcept
      : id_{std::move(id)}, name_{std::move(name)}, dog_speed_{dog_speed},
        randomize_spawn_point_{randomize_spawn_point} {}

  const Id &GetId() const noexcept;
  const std::string &GetName() const noexcept;
  const Buildings &GetBuildings() const noexcept;
  const Roads &GetRoads() const noexcept;
  const Offices &GetOffices() const noexcept;
  const detail::Speed GetDogSpeed() const;
  const PairRoads GetRoadsByCoordinate(const detail::Coordinate &point,
                                       detail::Direction direction) const;

  const detail::Point GetDefaultDogPosition() const;
  detail::Point RandomStart() const;

  void AddRoad(Road road);
  void AddBuilding(Building building);
  void AddOffice(Office office);

private:
  const Road *GetRoadPtr(const Map::RoadsBaseIterator &it,
                         detail::DogCoord coord, detail::Direction direction,
                         Map::GetCoordsMethod get_coords) const;

  std::pair<detail::Coord, detail::Coord>
  GetYCoords(const Road *road_ptr) const;

  std::pair<detail::Coord, detail::Coord>
  GetXCoords(const Road *road_ptr) const;

private:
  using OfficeIdToIndex =
      std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

  Id id_;
  std::string name_;
  detail::Speed dog_speed_;

  bool randomize_spawn_point_;

  Roads roads_;
  Buildings buildings_;

  OfficeIdToIndex warehouse_id_to_index_;
  Offices offices_;

  RoadsBase horizontal_roads_;
  RoadsBase vertical_roads_;
};
} // namespace model