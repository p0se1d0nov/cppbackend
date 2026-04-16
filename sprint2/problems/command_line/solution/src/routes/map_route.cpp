#include "map_route.h"
#include "game.h"
#include "request_constant_values.h"

namespace Routes {

using namespace std::literals;
using namespace model;

http_handler::LogResponseData MapRoute::operator()(const It begin, const It end,
                                                   const Method method,
                                                   const Request &req) const {
  if (begin == end) {
    return method(
        http::status::ok,
        boost::json::parse(boost::json::serialize(GetArrayJsonMapHeads())),
        ""sv);
  }

  const auto id = Map::Id(std::string(*begin));
  const auto map_ptr = this->game_.FindMap(id);
  if (!map_ptr) {
    return method(http::status::not_found, MAP_NOT_FOUND, ""sv);
  }
  const auto json_map = GetObjectJsonFromMap(map_ptr);
  return method(http::status::ok,
                boost::json::parse(boost::json::serialize(json_map)), ""sv);
};

boost::json::array MapRoute::GetArrayJsonMapHeads() const {
  const auto maps = this->game_.GetMaps();
  boost::json::array result_maps;
  for (const auto &map : maps) {
    result_maps.push_back({{"id"s, *map.GetId()}, {"name"s, map.GetName()}});
  }
  return result_maps;
}

boost::json::object MapRoute::GetObjectJsonFromMap(const Map *const map) const {
  boost::json::object json_map = GreateJsonMap(map);

  AddRoadsInJsonObject(json_map, map);
  AddBuildingsInJsonObject(json_map, map);
  AddOfficesInJsonObject(json_map, map);

  return json_map;
}

boost::json::object MapRoute::GreateJsonMap(const Map *const map) const {
  const auto map_id = map->GetId();
  boost::json::object json_map;
  json_map["id"s] = *map_id;
  json_map["name"s] = map->GetName();
  return json_map;
}

void MapRoute::AddRoadsInJsonObject(boost::json::object &json_map,
                                    const Map *const map) const {
  json_map["roads"] = boost::json::array{};
  for (const auto &road : map->GetRoads()) {
    boost::json::object road_object{};

    const auto [s_x, s_y] = road.GetJsonStart();
    road_object["x0"] = s_x;
    road_object["y0"] = s_y;

    const auto [e_x, e_y] = road.GetJsonEnd();
    if (road.IsHorizontal()) {
      road_object["x1"] = e_x;
    } else {
      road_object["y1"] = e_y;
    }

    json_map["roads"].as_array().emplace_back(std::move(road_object));
  }
}

void MapRoute::AddBuildingsInJsonObject(boost::json::object &json_map,
                                        const Map *const map) const {
  json_map["buildings"] = boost::json::array{};

  for (const auto &building : map->GetBuildings()) {
    boost::json::object building_object{};

    const auto &[position, size] = building.GetBounds();
    const auto [x, y] = position;
    const auto [w, h] = size;
    building_object["x"] = x;
    building_object["y"] = y;
    building_object["w"] = w;
    building_object["h"] = h;

    json_map["buildings"].as_array().emplace_back(std::move(building_object));
  }
}

void MapRoute::AddOfficesInJsonObject(boost::json::object &json_map,
                                      const Map *const map) const {
  json_map["offices"] = boost::json::array{};
  for (const auto &office : map->GetOffices()) {
    boost::json::object office_object{};

    office_object["id"] = *office.GetId();

    const auto [x, y] = office.GetPosition();
    office_object["x"] = x;
    office_object["y"] = y;

    const auto &[dx, dy] = office.GetOffset();
    office_object["offsetX"] = dx;
    office_object["offsetY"] = dy;

    json_map["offices"].as_array().emplace_back(std::move(office_object));
  }
}

} // namespace Routes