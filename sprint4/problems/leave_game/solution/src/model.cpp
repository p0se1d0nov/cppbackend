#include "model.h"
#include <stdexcept>
#include <chrono>
#include <random>
#include <iostream>

namespace model {
using namespace std::literals;


void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId()))
        throw std::invalid_argument("Duplicate warehouse");
    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        offices_.pop_back();
        throw;
    }
}

int64_t Map::FindRoadIndex(double x, double y, std::unordered_set<size_t>& viewed_road) const {
    for (size_t i = 0; i < roads_.size(); ++i) {
        if (viewed_road.count(i)) continue;
        const auto& road = roads_.at(i);
        double min_x = std::min(road.GetStart().x, road.GetEnd().x) - ROAD_WIGHT;
        double min_y = std::min(road.GetStart().y, road.GetEnd().y) - ROAD_WIGHT;
        double max_x = std::max(road.GetStart().x, road.GetEnd().x) + ROAD_WIGHT;
        double max_y = std::max(road.GetStart().y, road.GetEnd().y) + ROAD_WIGHT;
        if (x >= min_x && x <= max_x && y >= min_y && y <= max_y) {
            viewed_road.insert(i);
            return i;
        }
    }
    return -1;
}

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

const Map* Game::FindMap(const Map::Id& id) const noexcept {
    auto it = map_id_to_index_.find(id);
    return it != map_id_to_index_.end() ? &maps_.at(it->second) : nullptr;
}

GameSession& Game::GetOrCreateSession(const Map::Id& map_id) {
    auto it = std::find_if(sessions_.begin(), sessions_.end(),
                           [&](const std::unique_ptr<GameSession>& s) { return s->GetMap().GetId() == map_id; });
    if (it != sessions_.end()) return **it;
    auto map = FindMap(map_id);
    if (!map) throw std::invalid_argument("Map not found");
    sessions_.push_back(std::make_unique<GameSession>(*map, random_spawn_,dogRetirementTime_));
    return *sessions_.back();
}

void Game::UpdateState(std::chrono::milliseconds delta) {
    for (auto& session : sessions_) {
        session->UpdateState(delta);
    }
}

std::pair<double, double> GameSession::GenerateRandomPosition() {
    const auto& roads = map_.GetRoads();
    if (roads.empty()) return {0.0, 0.0};
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> road_dist(0, roads.size() - 1);
    const auto& road = roads[road_dist(gen)];
    if (road.IsHorizontal()) {
        double x1 = std::min(road.GetStart().x, road.GetEnd().x);
        double x2 = std::max(road.GetStart().x, road.GetEnd().x);
        std::uniform_real_distribution<double> coord_dist(x1, x2);
        return {coord_dist(gen), static_cast<double>(road.GetStart().y)};
    } else {
        double y1 = std::min(road.GetStart().y, road.GetEnd().y);
        double y2 = std::max(road.GetStart().y, road.GetEnd().y);
        std::uniform_real_distribution<double> coord_dist(y1, y2);
        return {static_cast<double>(road.GetStart().x), coord_dist(gen)};
    }
}

Dog& GameSession::AddDogRandom(std::string name) {
    auto id = next_dog_id_++;
    auto dog = std::make_unique<Dog>(id, std::move(name));
    auto [x, y] = GenerateRandomPosition();
    dog->SetPosition(x, y);
    dog->SetDirection(Direction::NORTH);
    Dog* ptr = dog.get();
    dogs_.push_back(std::move(dog));
    return *ptr;
}

Dog& GameSession::AddDogAtZeroPoint(std::string name) {
    auto id = next_dog_id_++;
    auto dog = std::make_unique<Dog>(id, std::move(name));
    const auto& roads = map_.GetRoads();
    if (!roads.empty()) {
        const auto& road = roads[0];
        dog->SetPosition(road.GetStart().x, road.GetStart().y);
    } else {
        dog->SetPosition(0.0, 0.0);
    }
    dog->SetDirection(Direction::NORTH);
    Dog* ptr = dog.get();
    dogs_.push_back(std::move(dog));
    return *ptr;
}

Dog& GameSession::AddDog(std::string name) {
    if (random_spawn_) return AddDogRandom(name);
    return AddDogAtZeroPoint(name);
}

void GameSession::UpdateState(std::chrono::milliseconds delta) {
    // движение собак
    std::vector<geom::Point2D> old_pos;

    for (const auto& dog : dogs_) {
        if (dog->GetVx() == 0.0 && dog->GetVy() == 0){  // Нахождение AFK собак
            dog->UpdateAFKTime(static_cast<double>(delta.count())); // Увеличиваем время
            if (!dog->IsDeleted() && dog->GetAFKTime() >= dogRetirementTime_) {
                dog->MarkDeleted();  // Помечаем собаку удаленной
            }
        }
        else {
            dog->ResetAFKTime();  // Сбрасываем время АФК
        }
        dog->UpdateActiveTime(delta.count());
    }

    for (const auto& dog : dogs_) {
        old_pos.emplace_back(dog->GetX(), dog->GetY());
    }

    MoveDogs(delta);
    ModelCollisionProvider provider(dogs_, old_pos, loot_, map_.GetOffices());
    auto events = collision_detector::FindGatherEvents(provider);

    if (events.size() > 0) {

    }

    for (const auto& event: events) {
        auto& dog = dogs_[event.gatherer_id];
        if (event.item_id < loot_.size()) {      // обрабатываем предмет
            const auto& loot = loot_[event.item_id];
            if (!dog->IsBagFull(map_.GetBagCapacity()) && loot.on_map ) {
                dog->AddToBag(loot);
                PutLootInBag(event.item_id);
            }
        }
        else {
            dog->ClearBag();
        }
    }


    std::erase_if(loot_, [](Loot& l) { return l.on_map == false; });
    // генерация лута
    AddRandomLoot(delta);
}

void GameSession::AddLoot(Loot loot) {
    loot_.push_back(std::move(loot));
}

void GameSession::RemoveDeletedDogs()
{
    dogs_.erase(
        std::remove_if(dogs_.begin(), dogs_.end(),
                       [](const std::unique_ptr<Dog>& dog) {
                           return dog->IsDeleted();
                       }),
        dogs_.end()
        );
}

void GameSession::MoveDogs(std::chrono::milliseconds delta)
{
    double time_s = std::chrono::duration<double>(delta).count();
    for (auto& dog_ : dogs_) {
        double vx = dog_->GetVx(), vy = dog_->GetVy();

        if (vx == 0.0 && vy == 0.0) continue;
        double x = dog_->GetX(), y = dog_->GetY();
        double next_x = x + vx * time_s;
        double next_y = y + vy * time_s;
        const auto& roads = map_.GetRoads();
        auto road_it = std::find_if(roads.begin(), roads.end(),
                                    [next_x, next_y](const Road& road) {
                                        double min_x = std::min(road.GetStart().x, road.GetEnd().x) - ROAD_WIGHT;
                                        double min_y = std::min(road.GetStart().y, road.GetEnd().y) - ROAD_WIGHT;
                                        double max_x = std::max(road.GetStart().x, road.GetEnd().x) + ROAD_WIGHT;
                                        double max_y = std::max(road.GetStart().y, road.GetEnd().y) + ROAD_WIGHT;
                                        return (next_x >= min_x && next_x <= max_x && next_y >= min_y && next_y <= max_y);
                                    });
        if (road_it != roads.end()) {
            dog_->SetPosition(next_x, next_y);
            continue;
        }
        next_x = x; next_y = y;
        std::unordered_set<size_t> viewed;
        while (true) {
            int64_t idx = map_.FindRoadIndex(next_x, next_y, viewed);
            if (idx == -1) break;
            const auto& road = roads.at(idx);
            switch (dog_->GetDirection()) {
            case Direction::NORTH: next_y = std::min(road.GetStart().y, road.GetEnd().y) - ROAD_WIGHT; break;
            case Direction::SOUTH: next_y = std::max(road.GetStart().y, road.GetEnd().y) + ROAD_WIGHT; break;
            case Direction::WEST:  next_x = std::min(road.GetStart().x, road.GetEnd().x) - ROAD_WIGHT; break;
            case Direction::EAST:  next_x = std::max(road.GetStart().x, road.GetEnd().x) + ROAD_WIGHT; break;
            }
        }
        dog_->SetSpeed(0.0, 0.0);
        dog_->SetPosition(next_x, next_y);


    }
}


void GameSession::AddRandomLoot(std::chrono::milliseconds dt) {
    unsigned loot_count = static_cast<unsigned>(loot_.size());
    unsigned looter_count = static_cast<unsigned>(dogs_.size());
    unsigned new_loot = loot_generator_.Generate(dt, loot_count, looter_count);
    for (unsigned i = 0; i < new_loot; ++i) {
        SpawnOneLoot();
    }
}

void GameSession::SpawnOneLoot() {
    Loot loot;
    loot.id = next_loot_id_++;
    loot.type = GenerateRandomLootType();
    Position pos = GenerateRandomPositionOnRoad();
    loot.x = pos.x;
    loot.y = pos.y;
    loot.value = map_.GetLootTypeValue(loot.type);
    loot_.push_back(loot);
}

uint8_t GameSession::GenerateRandomLootType() {
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(map_.GetLootTypesCount()) - 1);
    return static_cast<uint8_t>(dist(gen));
}

Position GameSession::GenerateRandomPositionOnRoad() {
    const auto& roads = map_.GetRoads();
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> road_dist(0, roads.size() - 1);
    const Road& r = roads[road_dist(gen)];
    return r.GetRandomPoint();
}

ModelCollisionProvider::ModelCollisionProvider(const std::vector<std::unique_ptr<Dog> > &dogs
                                               , const std::vector<geom::Point2D> &old_positions
                                               , const std::vector<Loot> &loot
                                               , const std::vector<Office> &offices)
                                                : dogs_(dogs)
                                                , old_positions_(old_positions)
                                                , loot_(loot), offices_(offices) {}

size_t ModelCollisionProvider::ItemsCount() const {
    return loot_.size() + offices_.size();

}

collision_detector::Item ModelCollisionProvider::GetItem(size_t idx) const {
    if (idx < loot_.size()) {
        const auto& l = loot_[idx];
        return { {l.x, l.y}, ITEM_WIDTH };
    }
    const auto& office = offices_[idx - loot_.size()];
    return { {static_cast<double>(office.GetPosition().x),
              static_cast<double>(office.GetPosition().y)}, OFFICE_WIDTH };
}

size_t ModelCollisionProvider::GatherersCount() const {
    return dogs_.size();
}

collision_detector::Gatherer ModelCollisionProvider::GetGatherer(size_t idx) const {
    const auto& dog = dogs_[idx];
    const auto& start = old_positions_[idx];
    geom::Point2D end{dog->GetX(), dog->GetY()};
    return { start, end, DOG_WIDTH };
}


} // namespace model
