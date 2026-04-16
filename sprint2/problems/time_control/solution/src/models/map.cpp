#include "map.h"
#include "details.h"
#include "road.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <iterator>
#include <random>
#include <stdexcept>

namespace model {
const Map::Id &Map::GetId() const noexcept { return id_; }
const std::string &Map::GetName() const noexcept { return name_; }
const Map::Buildings &Map::GetBuildings() const noexcept { return buildings_; }
const Map::Roads &Map::GetRoads() const noexcept { return roads_; }
const Map::Offices &Map::GetOffices() const noexcept { return offices_; }
const detail::Speed Map::GetDogSpeed() const { return dog_speed_; };

const Map::PairRoads
Map::GetRoadsByCoordinate(const detail::Coordinate &point,
                          detail::Direction direction) const {
  const auto [x, y] = point;
  const auto h_it = horizontal_roads_.find(std::round(y));
  const auto v_it = vertical_roads_.find(std::round(x));
  const Road *h_road = nullptr, *v_road = nullptr;

  if (h_it != horizontal_roads_.end()) {
    h_road = GetRoadPtr(h_it, x, direction, &Map::GetXCoords);
  }

  if (v_it != vertical_roads_.end()) {
    v_road = GetRoadPtr(v_it, y, direction, &Map::GetYCoords);
  }

  return {v_road, h_road};
};

std::pair<detail::Coord, detail::Coord>
Map::GetYCoords(const Road *road_ptr) const {
  auto [_, s_y] = road_ptr->GetStart();
  auto [__, e_y] = road_ptr->GetEnd();
  return {s_y, e_y};
};

std::pair<detail::Coord, detail::Coord>
Map::GetXCoords(const Road *road_ptr) const {
  auto [s_x, _] = road_ptr->GetStart();
  auto [e_x, __] = road_ptr->GetEnd();
  return {s_x, e_x};
};

const Road *Map::GetRoadPtr(const Map::RoadsBaseIterator &it,
                            detail::DogCoord coord, detail::Direction direction,
                            Map::GetCoordsMethod get_coords) const {
  const auto &roads_map = it->second;
  auto map_it = roads_map.lower_bound(coord);
  const Road *road = nullptr;
  if (map_it != roads_map.end()) {
    const auto road_ptr = map_it->second;
    assert(road_ptr);
    const Road *prev_road_ptr = nullptr;
    const Road *next_road_ptr = nullptr;
    const auto [start, end] = (this->*get_coords)(road_ptr);
    bool is_current = start - detail::ROAD_OFFSET <= coord &&
                      end + detail::ROAD_OFFSET >= coord;
    bool is_prev = false;
    bool is_next = false;

    if (map_it != roads_map.begin()) {
      prev_road_ptr = std::prev(map_it)->second;
      const auto [start, end] = (this->*get_coords)(prev_road_ptr);
      is_prev = start - detail::ROAD_OFFSET <= coord &&
                end + detail::ROAD_OFFSET >= coord;
    }

    if (std::next(map_it) != roads_map.end()) {
      next_road_ptr = std::next(map_it)->second;
      const auto [start, end] = (this->*get_coords)(next_road_ptr);
      is_next = start - detail::ROAD_OFFSET <= coord &&
                end + detail::ROAD_OFFSET >= coord;
    }

    if (direction == detail::Direction::SOUTH ||
        direction == detail::Direction::EAST) {
      if (is_prev) {
        road = prev_road_ptr;
      }
      if (is_current) {
        road = road_ptr;
      }
      if (is_next) {
        road = next_road_ptr;
      }
    }
    if (direction == detail::Direction::NORTH ||
        direction == detail::Direction::WEST) {
      if (is_next) {
        road = next_road_ptr;
      }
      if (is_current) {
        road = road_ptr;
      }
      if (is_prev) {
        road = prev_road_ptr;
      }
    }
  } else {
    const auto last_road_ptr = roads_map.rbegin()->second;
    const auto [_, end] = (this->*get_coords)(last_road_ptr);
    if (coord <= end + detail::ROAD_OFFSET) {
      road = last_road_ptr;
    }
  }

  return road;
}

void Map::AddRoad(Road t_road) {
  const auto &road = roads_.emplace_back(std::move(t_road));
  const auto [x, y] = road.GetStart();
  if (road.IsHorizontal()) {
    const auto it = horizontal_roads_.find(y);
    if (it == horizontal_roads_.end()) {
      RoadsMap tmp_map;
      tmp_map.emplace(x, &road);
      horizontal_roads_.emplace(y, std::move(tmp_map));
    } else {
      it->second.emplace(x, &road);
    }

  } else {
    const auto it = vertical_roads_.find(x);
    if (it == vertical_roads_.end()) {
      RoadsMap tmp_map;
      tmp_map.emplace(y, &road);
      vertical_roads_.emplace(x, std::move(tmp_map));
    } else {
      it->second.emplace(y, &road);
    }
  }
}

void Map::AddBuilding(Building building) {
  buildings_.emplace_back(std::move(building));
}

void Map::AddOffice(Office office) {
  if (warehouse_id_to_index_.contains(office.GetId())) {
    throw std::invalid_argument("Duplicate warehouse");
  }

  const size_t index = offices_.size();
  Office &o = offices_.emplace_back(std::move(office));
  try {
    warehouse_id_to_index_.emplace(o.GetId(), index);
  } catch (...) {
    offices_.pop_back();
    throw;
  }
}

const detail::Point Map::GetDefaultDogPosition() const {
  return randomize_spawn_point_ ? RandomStart() : GetRoads().at(0).GetStart();
}

detail::Point Map::RandomStart() const {
  static std::mt19937 gen(
      std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<> random_road_index(0, roads_.size() - 1);

  const auto &road = roads_.at(random_road_index(gen));

  const auto [s_x, s_y] = road.GetStart();
  const auto [e_x, e_y] = road.GetEnd();

  if (road.IsHorizontal()) {
    std::uniform_int_distribution<> random_point_index(s_x, e_x);
    const auto x = random_point_index(gen);
    return {x, e_y};
  }
  std::uniform_int_distribution<> random_point_index(s_y, e_y);
  const auto y = random_point_index(gen);
  return {e_x, y};
}

}; // namespace model