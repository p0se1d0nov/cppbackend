#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

bool Road::IsHorizontal() const noexcept { return start_.y == end_.y; }
bool Road::IsVertical() const noexcept { return start_.x == end_.x; }
Point Road::GetStart() const noexcept { return start_; }
Point Road::GetEnd() const noexcept { return end_; }

const Rectangle &Building::GetBounds() const noexcept { return bounds_; }

const Office::Id &Office::GetId() const noexcept { return id_; }
Point Office::GetPosition() const noexcept { return position_; }
Offset Office::GetOffset() const noexcept { return offset_; }

const Map::Id &Map::GetId() const noexcept { return id_; }
const std::string &Map::GetName() const noexcept { return name_; }
const Map::Buildings &Map::GetBuildings() const noexcept { return buildings_; }
const Map::Roads &Map::GetRoads() const noexcept { return roads_; }
const Map::Offices &Map::GetOffices() const noexcept { return offices_; }
void Map::AddRoad(const Road &road) { roads_.emplace_back(road); }
void Map::AddBuilding(const Building &building) {
  buildings_.emplace_back(building);
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
    // Удаляем офис из вектора, если не удалось вставить в unordered_map
    offices_.pop_back();
    throw;
  }
}

void Game::AddMap(Map map) {
  const size_t index = maps_.size();
  if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index);
      !inserted) {
    throw std::invalid_argument("Map with id "s + *map.GetId() +
                                " already exists"s);
  } else {
    try {
      maps_.emplace_back(std::move(map));
    } catch (...) {
      map_id_to_index_.erase(it);
      throw;
    }
  }
}

const Game::Maps &Game::GetMaps() const noexcept { return maps_; }

const Map *Game::FindMap(const Map::Id &id) const noexcept {
  if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
    return &maps_.at(it->second);
  }
  return nullptr;
}

} // namespace model
