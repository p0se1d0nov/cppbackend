#include "json_loader.h"
#include "model.h"
#include "tagged.h"
#include <boost/json.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>

namespace json_loader {
using namespace std::literals;
using namespace model;
namespace property_tree = boost::property_tree;
using ptree = property_tree::ptree;

using ItemJson =
    const std::pair<const std::string,
                    property_tree::basic_ptree<std::string, std::string>>;

Map CreateMap(ItemJson &item) {
  const auto id = item.second.get<std::string>("id");
  const auto name = item.second.get<std::string>("name");

  auto map_id = Map::Id(std::move(id));
  return Map(std::move(map_id), std::move(name));
}

void AddRoadsToMap(ItemJson &item, Map &map) {
  const auto roads = item.second.get_child_optional("roads");

  if (roads) {
    for (const auto &[_, road] : *roads) {
      const auto x0 = road.get<int>("x0");
      const auto y0 = road.get<int>("y0");
      const auto x1 = road.get_optional<int>("x1");
      const auto y1 = road.get_optional<int>("y1");

      if (x1) {
        const Road road_s{Road::HORIZONTAL, {x0, y0}, *x1};
        map.AddRoad(std::move(road_s));
      } else {
        const Road road_s{Road::VERTICAL, {x0, y0}, *y1};
        map.AddRoad(std::move(road_s));
      }
    }
  }
}

void AddBuildingsToMap(ItemJson &item, Map &map) {

  const auto buildings = item.second.get_child_optional("buildings");
  if (buildings) {
    for (const auto &[_, building] : *buildings) {
      const auto x = building.get<int>("x");
      const auto y = building.get<int>("y");
      const auto w = building.get<int>("w");
      const auto h = building.get<int>("h");

      const Building building_s{{{x, y}, {w, h}}};
      map.AddBuilding(std::move(building_s));
    }
  }
}

void AddOfficesToMap(ItemJson &item, Map &map) {
  const auto offices = item.second.get_child_optional("offices");

  if (offices) {
    for (const auto &[_, office] : *offices) {
      auto id = office.get<std::string>("id");
      const auto x = office.get<int>("x");
      const auto y = office.get<int>("y");
      const auto off_x = office.get<int>("offsetX");
      const auto off_y = office.get<int>("offsetY");

      auto tagged_office_id = Office::Id(std::move(id));

      const Office office_s{tagged_office_id, {x, y}, {off_x, off_y}};
      map.AddOffice(std::move(office_s));
    }
  }
}

void AddMapsFromJSONTree(Game &game, const ptree &pt) {
  auto root_json = pt.get_child_optional("maps");
  if (root_json) {
    for (const auto &item_json : *root_json) {
      Map map = CreateMap(item_json);

      AddRoadsToMap(item_json, map);
      AddBuildingsToMap(item_json, map);
      AddOfficesToMap(item_json, map);

      game.AddMap(std::move(map));
    }
  }
}

Game LoadGame(const std::filesystem::path &json_path) {
  // Загрузить содержимое файла json_path, например, в виде строки
  // Распарсить строку как JSON, используя boost::json::parse
  // Загрузить модель игры из файла

  ptree pt;

  try {
    property_tree::read_json(json_path, pt);
  } catch (const property_tree::json_parser_error &e) {
    // std::cerr << "JSON not valid" << e.what() << std::endl;
    throw;
  }

  Game game;

  AddMapsFromJSONTree(game, pt);

  return game;
}

} // namespace json_loader
