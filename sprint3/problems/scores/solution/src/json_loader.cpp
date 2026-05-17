#include "json_loader.h"
#include <boost/json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include "extradata.h"

namespace json_loader {

void LoadRoads(model::Map& map, const boost::json::object& map_obj) {
    for (auto& road_val : map_obj.at("roads").as_array()) {
        auto& road_obj = road_val.as_object();
        if (road_obj.contains("x1")) {
            int x0 = road_obj.at("x0").as_int64();
            int y0 = road_obj.at("y0").as_int64();
            int x1 = road_obj.at("x1").as_int64();
            map.AddRoad(model::Road(model::Road::HORIZONTAL, {x0, y0}, x1));
        } else {
            int x0 = road_obj.at("x0").as_int64();
            int y0 = road_obj.at("y0").as_int64();
            int y1 = road_obj.at("y1").as_int64();
            map.AddRoad(model::Road(model::Road::VERTICAL, {x0, y0}, y1));
        }
    }
}

void LoadBuildings(model::Map& map, const boost::json::object& map_obj) {
    for (auto& building_val : map_obj.at("buildings").as_array()) {
        auto& b = building_val.as_object();
        int x = b.at("x").as_int64();
        int y = b.at("y").as_int64();
        int w = b.at("w").as_int64();
        int h = b.at("h").as_int64();
        map.AddBuilding(model::Building({{x, y}, {w, h}}));
    }
}

void LoadOffices(model::Map& map, const boost::json::object& map_obj) {
    for (auto& office_val : map_obj.at("offices").as_array()) {
        auto& o = office_val.as_object();
        std::string office_id = boost::json::value_to<std::string>(o.at("id"));
        int x = o.at("x").as_int64();
        int y = o.at("y").as_int64();
        int offsetX = o.at("offsetX").as_int64();
        int offsetY = o.at("offsetY").as_int64();
        map.AddOffice(model::Office(
            model::Office::Id(std::move(office_id)),
            {x, y},
            {offsetX, offsetY}
        ));
    }
}

void LoadLoot(model::Map& map, const boost::json::object& map_obj) {
    const auto& loot_arr = map_obj.at("lootTypes").as_array();
    map.SetLootTypesCount(loot_arr.size());
    extra_data::ExtraDataRepository::GetInstance().SetLootTypes(map.GetId(), loot_arr);
    uint8_t type = 0;
    for (auto& loot_val : loot_arr) {
        auto& l = loot_val.as_object();
        map.SetLootTypeValue(type++,l.at("value").as_int64());
    }
}

void LoadDogSpeed(model::Map& map, const boost::json::object& map_obj) {
    double speed = map_obj.at("dogSpeed").as_double();
    map.SetDogSpeed(speed);
}

model::Map LoadMap(const boost::json::object& map_obj,
                   double default_speed,
                   const loot_gen::LootGenerator& generator) {
    std::string id = boost::json::value_to<std::string>(map_obj.at("id"));
    std::string name = boost::json::value_to<std::string>(map_obj.at("name"));
    model::Map map{model::Map::Id(std::move(id)), std::move(name)};

    if (map_obj.contains("roads")) LoadRoads(map, map_obj);
    if (map_obj.contains("buildings")) LoadBuildings(map, map_obj);
    if (map_obj.contains("offices")) LoadOffices(map, map_obj);

    if (map_obj.contains("dogSpeed")) LoadDogSpeed(map, map_obj);
    else map.SetDogSpeed(default_speed);

    if (map_obj.contains("defaultBagCapacity")) {
        int cap = map_obj.at("defaultBagCapacity").get_uint64();
        map.SetBagCapacity(cap);
    }

    if (map_obj.contains("lootTypes")) {
        LoadLoot(map,map_obj);
    } else {
        throw std::runtime_error("Map missing lootTypes");
    }

    map.SetLootGenerator(generator);
    return map;
}

boost::json::value LoadJSON(const std::filesystem::path& json_path) {
    namespace fs = std::filesystem;
    fs::path abs_path = fs::absolute(json_path).lexically_normal();
    if (!fs::exists(abs_path)) {
        throw std::runtime_error("Config file not found");
    }
    std::ifstream f(abs_path);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open config file: " + json_path.string());
    }
    std::stringstream ss;
    ss << f.rdbuf();
    std::string data = ss.str();
    try {
        return boost::json::parse(data);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse JSON config: " + std::string(e.what()));
    }
}

model::Game LoadGame(const std::filesystem::path& json_path, bool random_spawn) {
    model::Game game;
    game.SetDogRandomSpawn(random_spawn);

    auto config = LoadJSON(json_path);

    if (config.as_object().contains("defaultDogSpeed")) {
        game.SetDefaultDogSpeed(config.as_object().at("defaultDogSpeed").as_double());
    }

    // Параметры генератора
    std::chrono::milliseconds loot_period(5000);
    double loot_probability = 0.5;
    if (config.as_object().contains("lootGeneratorConfig")) {
        auto& loot_cfg = config.at("lootGeneratorConfig").as_object();
        double period_sec = loot_cfg.at("period").as_double();
        loot_probability = loot_cfg.at("probability").as_double();
        loot_period = std::chrono::milliseconds(static_cast<int64_t>(period_sec * 1000));
    }

    // Генератор случайных чисел для LootGenerator
    //static thread_local std::mt19937 rng(std::random_device{}());
    //auto random_gen = [&rng]() { return std::uniform_real_distribution<double>(0.0, 1.0)(rng); };
    loot_gen::LootGenerator default_generator(loot_period, loot_probability);

    double default_speed = game.GetDefaultDogSpeed();
    auto maps_arr = config.as_object().at("maps").as_array();
    for (auto& map_value : maps_arr) {
        auto map_obj = map_value.as_object();
        auto map = LoadMap(map_obj, default_speed, default_generator);
        game.AddMap(std::move(map));
    }

    return game;
}

} // namespace json_loader
