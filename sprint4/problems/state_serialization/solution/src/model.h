#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <memory>
#include <random>
#include <optional>
#include <unordered_set>

#include "tagged.h"
#include "loot_generator.h"

#include "collision_detector.h"


namespace model {

const double ROAD_WIGHT = 0.4;
const double COEF_SEC_TO_MILLISEC = 0.001;
const double ITEM_WIDTH = 0.0;
const double DOG_WIDTH =  0.6;
const double OFFICE_WIDTH = 0.5;

enum class Direction {
    NORTH, SOUTH, WEST, EAST
};

using Dimension = int;
using Coord = Dimension;

struct Point { Coord x, y; };
struct Size { Dimension width, height; };
struct Rectangle { Point position; Size size; };
struct Offset { Dimension dx, dy; };
struct Position { double x, y; };


class Road {
    struct HorizontalTag { explicit HorizontalTag() = default; };
    struct VerticalTag { explicit VerticalTag() = default; };
public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept : start_{start}, end_{end_x, start.y} {}
    Road(VerticalTag, Point start, Coord end_y) noexcept : start_{start}, end_{start.x, end_y} {}

    bool IsHorizontal() const noexcept { return start_.y == end_.y; }
    bool IsVertical() const noexcept { return start_.x == end_.x; }
    Point GetStart() const noexcept { return start_; }
    Point GetEnd() const noexcept { return end_; }

    Position GetRandomPoint() const {
        static thread_local std::mt19937 gen(std::random_device{}());
        if (IsHorizontal()) {
            std::uniform_real_distribution<double> dist(std::min(start_.x, end_.x), std::max(start_.x, end_.x));
            return Position{dist(gen), static_cast<double>(start_.y)};
        } else {
            std::uniform_real_distribution<double> dist(std::min(start_.y, end_.y), std::max(start_.y, end_.y));
            return Position{static_cast<double>(start_.x), dist(gen)};
        }
    }

private:
    Point start_, end_;
};


class Building {
public:
    explicit Building(Rectangle bounds) noexcept : bounds_(bounds) {}
    const Rectangle& GetBounds() const noexcept { return bounds_; }
private:
    Rectangle bounds_;
};


class Office {
public:
    using Id = util::Tagged<std::string, Office>;
    Office(Id id, Point position, Offset offset) noexcept : id_(std::move(id)), position_(position), offset_(offset) {}
    const Id& GetId() const noexcept { return id_; }
    Point GetPosition() const noexcept { return position_; }
    Offset GetOffset() const noexcept { return offset_; }
private:
    Id id_;
    Point position_;
    Offset offset_;
};


struct Loot {
    size_t id;
    uint8_t type;
    double x, y;
    double value;
    bool on_map = true;
};


class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept : id_(std::move(id)), name_(std::move(name)) {}

    const Id& GetId() const noexcept { return id_; }
    const std::string& GetName() const noexcept { return name_; }
    const Buildings& GetBuildings() const noexcept { return buildings_; }
    const Roads& GetRoads() const noexcept { return roads_; }
    const Offices& GetOffices() const noexcept { return offices_; }

    void AddRoad(const Road& road) {
        size_t index = roads_.size();
        roads_.emplace_back(road);
        if (road.IsHorizontal()) horizontal_roads[road.GetStart().y].push_back(index);
        else vertical_roads[road.GetStart().x].push_back(index);
    }
    void AddBuilding(const Building& building) { buildings_.emplace_back(building); }
    void AddOffice(Office office);

    void SetDogSpeed(double speed) { dogSpeed_ = speed; }
    double GetDogSpeed() const { return dogSpeed_; }

    int64_t FindRoadIndex(double x, double y, std::unordered_set<size_t>& viewed_road) const;

    void SetLootTypesCount(size_t count) { loot_types_count_ = count; }
    size_t GetLootTypesCount() const { return loot_types_count_; }

    void SetLootGenerator(loot_gen::LootGenerator generator) { generator_ = std::move(generator); }
    const loot_gen::LootGenerator& GetLootGenerator() const {
        if (!generator_) throw std::runtime_error("LootGenerator not set");
        return *generator_;
    }

    void SetLootTypeValue(uint8_t type, int value) {
        loot_types_value_.insert({type,value});
    }

    int GetLootTypeValue(uint8_t type) const{
        return loot_types_value_.at(type);
    }

    int GetBagCapacity() const noexcept { return bag_size_; }
    void SetBagCapacity(int size) noexcept { bag_size_ = size; }

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    std::unordered_map<int, std::vector<size_t>> horizontal_roads, vertical_roads;
    Buildings buildings_;
    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
    double dogSpeed_ = 1;
    size_t loot_types_count_ = 0;
    std::unordered_map<uint8_t, int> loot_types_value_;
    std::optional<loot_gen::LootGenerator> generator_;
    int bag_size_ = 3;  // defailt value
};


class Dog {
public:
    using Id = uint64_t;
    Dog(Id id, std::string name) : id_(id), name_(std::move(name)), x_(0.0), y_(0.0), vx_(0.0), vy_(0.0), dir_(Direction::NORTH) {}

    Id GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    double GetX() const { return x_; }
    double GetY() const { return y_; }
    double GetVx() const { return vx_; }
    double GetVy() const { return vy_; }
    Direction GetDirection() const { return dir_; }

    void SetPosition(double x, double y) { x_ = x; y_ = y; }
    void SetSpeed(double vx, double vy) { vx_ = vx; vy_ = vy; }
    void SetDirection(Direction dir) { dir_ = dir; }

    void AddToBag(const Loot& loot) {
        bag_.push_back(loot);
        score_ += loot.value;
    }

    int GetScore() const {return score_;}
    void ClearBag() {bag_.clear();}
    bool IsBagFull(int max_size)  {return (bag_.size() >= max_size);}
    const std::vector<Loot>& GetBag() const {return bag_;}

private:
    Id id_;
    std::string name_;
    double x_, y_;
    double vx_, vy_;
    Direction dir_;
    int score_ = 0;
    std::vector<Loot> bag_;
};


class ModelCollisionProvider : public collision_detector::ItemGathererProvider {
public:
    ModelCollisionProvider(const std::vector<std::unique_ptr<Dog>>& dogs,
                           const std::vector<geom::Point2D>& old_positions,
                           const std::vector<Loot>& loot,
                           const std::vector<Office>& offices);

    size_t ItemsCount() const override;
    collision_detector::Item GetItem(size_t idx) const override;
    size_t GatherersCount() const override;
    collision_detector::Gatherer GetGatherer(size_t idx) const override;

private:
    const std::vector<std::unique_ptr<Dog>>& dogs_;
    const std::vector<geom::Point2D>& old_positions_;
    const std::vector<Loot>& loot_;
    const std::vector<Office>& offices_;
};


class GameSession {
public:
    explicit GameSession(const Map& map, bool random_spawn)
        : map_(map)
        , random_spawn_(random_spawn)
        , loot_generator_(map.GetLootGenerator())   // копируем генератор из карты
        , rng_(std::random_device{}())
    {}

    const Map& GetMap() const { return map_; }

    Dog& AddDogRandom(std::string name);
    Dog& AddDogAtZeroPoint(std::string name);
    Dog& AddDog(std::string name);

    void AddDog(std::unique_ptr<Dog> dog) {
        dogs_.push_back(std::move(dog));
    }

    void PutLootInBag(int id) {loot_[id].on_map = false;}

    void UpdateState(std::chrono::milliseconds delta);   // теперь delta в миллисекундах

    const std::vector<std::unique_ptr<Dog>>& GetDogs() const { return dogs_; }
    const std::vector<Loot>& GetLoot() const noexcept { return loot_; }

    void AddLoot(Loot loot);

    uint64_t GetNextDogId() const {
        return next_dog_id_;
    }

    uint64_t GetNextLootId() const {
        return next_loot_id_;
    }

    void SetNextDogId(uint64_t next_dog_id) {
        next_dog_id_ = next_dog_id;
    }

    void SetNextLootId(uint64_t next_loot_id) {
        next_loot_id_  = next_loot_id;
    }

    void ClearState() {
        dogs_.clear();
        loot_.clear();
        next_dog_id_ = 0;
        next_loot_id_ = 0;
    }

private:
    std::pair<double, double> GenerateRandomPosition();
    void MoveDogs(std::chrono::milliseconds delta);
    void AddRandomLoot(std::chrono::milliseconds dt);
    void SpawnOneLoot();
    uint8_t GenerateRandomLootType();
    Position GenerateRandomPositionOnRoad();

    const Map& map_;
    std::vector<std::unique_ptr<Dog>> dogs_;
    Dog::Id next_dog_id_ = 0;
    bool random_spawn_ = false;

    std::vector<Loot> loot_;
    uint64_t next_loot_id_ = 0;
    loot_gen::LootGenerator loot_generator_;   
    std::mt19937 rng_;   // для случайных позиций при спавне собак
};


class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map);
    const Maps& GetMaps() const noexcept { return maps_; }
    GameSession& GetOrCreateSession(const Map::Id& map_id);
    const Map* FindMap(const Map::Id& id) const noexcept;
    void SetDefaultDogSpeed(double speed) { defaultDogSpeed_ = speed; }
    double GetDefaultDogSpeed() const { return defaultDogSpeed_; }
    void UpdateState(std::chrono::milliseconds delta);
    void SetDogRandomSpawn(bool random) { random_spawn_ = random; }
    //void SetItemValue()

    std::vector<GameSession*> GetSessions() const {
        std::vector<GameSession*> result;
        for (const auto& s : sessions_) {
            result.push_back(s.get());
        }
        return result;
    }

    GameSession* FindSession(const Map::Id& id) {
        auto it = std::find_if(sessions_.begin(), sessions_.end(),
                               [&](const std::unique_ptr<GameSession>& s) { return s->GetMap().GetId() == id; });
        return it != sessions_.end() ? it->get() : nullptr;
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    double defaultDogSpeed_ = 1;
    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    std::vector<std::unique_ptr<GameSession>> sessions_;
    bool random_spawn_ = false;
};

} // namespace model
