#include "app.h"
#include <stdexcept>
#include "extradata.h"

namespace app {



Application::Application(model::Game& game, Players& players)
    : game_(game), players_(players) {}

Application::JoinGameResult Application::JoinGame(const std::string& user_name, const std::string& map_id_str) {
    model::Map::Id map_id(map_id_str);
    const auto* map = game_.FindMap(map_id);
    if (!map) {
        throw std::invalid_argument("Map not found");
    }
    auto& session = game_.GetOrCreateSession(map->GetId());

    auto& dog = session.AddDog(user_name);

    std::string token = players_.GenerateToken();
    auto player_id = players_.AddPlayer(token, dog, session);
    return {token, player_id};
}

std::vector<std::pair<Player::Id, std::string>> Application::GetPlayers(const std::string& token) {
    auto* player = players_.FindByToken(token);
    if (!player) {
        throw std::runtime_error("unknownToken");
    }
    auto& session = player->GetSession();
    const auto& dogs = session.GetDogs();
    std::vector<std::pair<Player::Id, std::string>> result;
    for (const auto& dog_ptr : dogs) {
        auto* p = players_.FindByDogId(dog_ptr->GetId());
        if (p) {
            result.emplace_back(p->GetId(), dog_ptr->GetName());
        }
    }
    return result;
}



std::vector<Application::PlayerState> Application::GetGameState(const std::string& token) {
    auto* player = players_.FindByToken(token);
    if (!player) {
        throw std::runtime_error("unknownToken");
    }
    auto& session = player->GetSession();
    const auto& dogs = session.GetDogs();
    std::vector<PlayerState> result;
    for (const auto& dog_ptr : dogs) {
        auto* p = players_.FindByDogId(dog_ptr->GetId());
        if (p) {
            result.push_back({p->GetId(),
                              dog_ptr->GetX(), dog_ptr->GetY(),
                              dog_ptr->GetVx(), dog_ptr->GetVy(),
                              dog_ptr->GetDirection()});
        }
    }
    return result;
}

const model::Game::Maps& Application::GetMaps() const {
    return game_.GetMaps();
}

const model::Map* Application::FindMap(const std::string& map_id_str) const {
    return game_.FindMap(model::Map::Id(map_id_str));
}

void Application::SetPlayerSpeed(const std::string &token, std::string str_dir) {
    auto* player = players_.FindByToken(token);
    if (!player) {
        throw std::runtime_error("unknownToken");
    }

    double vx = 0, vy = 0;
    if (str_dir.size() != 0){
        Direction dir;
        switch (str_dir[0]) {
        case 'U': dir = model::Direction::NORTH; break;
        case 'D': dir = model::Direction::SOUTH; break;
        case 'L': dir = model::Direction::WEST; break;
        case 'R': dir = model::Direction::EAST; break;
        default :
            throw InvalidArgument("invalidArgument");
            break;
        }

        player->GetDog().SetDirection(dir);


        double speed = player->GetSession().GetMap().GetDogSpeed();
        switch (dir) {
        case model::Direction::NORTH: vy = -speed; break;
        case model::Direction::SOUTH: vy = speed; break;
        case model::Direction::WEST:  vx = -speed ; break;
        case model::Direction::EAST:  vx = speed; break;
        }
    }

    player->GetDog().SetSpeed(vx,vy);
}

const std::vector<model::Loot>& Application::GetLoot(const std::string& token) const {
    auto* player = players_.FindByToken(token);
    if (!player) {
        throw std::runtime_error("unknownToken");
    }
    return player->GetSession().GetLoot();
}

boost::json::array Application::GetLootTypesForMap(const model::Map::Id& map_id) const {
    const auto* loot_types = extra_data::ExtraDataRepository::GetInstance().GetLootTypes(map_id);
    if (loot_types) {
        return *loot_types;
    }
    return boost::json::array();
}


} // namespace app
