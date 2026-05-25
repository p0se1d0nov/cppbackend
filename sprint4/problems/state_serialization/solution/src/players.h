#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include "player.h"
#include "token_generator.h"

namespace app {

// Хешер для pair<uint64_t, string>
struct PairHasher {
    size_t operator()(const std::pair<uint64_t, std::string>& p) const {
        return std::hash<uint64_t>()(p.first) ^ (std::hash<std::string>()(p.second) << 1);
    }
};

class Players {
public:
    using PlayerId = Player::Id;
    using MapId = std::string;

    PlayerId AddPlayer(std::string token, model::Dog& dog, model::GameSession& session) {
        PlayerId id = next_id_++;
        auto player = std::make_unique<Player>(id, std::move(token), dog, session);
        token_to_player_[player->GetToken()] = player.get();
        dog_to_player_[{dog.GetId(), *session.GetMap().GetId()}] = player.get();
        id_to_player_[id] = std::move(player);
        return id;
    }

    Player* FindByToken(const std::string& token) {
        auto it = token_to_player_.find(token);
        return it != token_to_player_.end() ? it->second : nullptr;
    }

    Player* FindById(PlayerId id) {
        auto it = id_to_player_.find(id);
        return it != id_to_player_.end() ? it->second.get() : nullptr;
    }

    // Поиск по dog_id и map_id (замена старому FindByDogId)
    Player* FindByDogIdAndMapId(model::Dog::Id dog_id, const MapId& map_id) {
        auto it = dog_to_player_.find({dog_id, map_id});
        return it != dog_to_player_.end() ? it->second : nullptr;
    }

    std::string GenerateToken() {
        return token_generator_.Generate();
    }

    std::vector<const Player*> GetAllPlayers() const {
        std::vector<const Player*> result;
        for (const auto& [id, player] : id_to_player_) {
            result.push_back(player.get());
        }
        return result;
    }

    void RestorePlayer(Player::Id id, std::string token, model::Dog& dog, model::GameSession& session) {
        if (id >= next_id_) {
            next_id_ = id + 1;
        }
        auto player = std::make_unique<Player>(id, std::move(token), dog, session);
        token_to_player_[player->GetToken()] = player.get();
        dog_to_player_[{dog.GetId(), *session.GetMap().GetId()}] = player.get();
        id_to_player_[id] = std::move(player);
    }

    void Clear() {
        id_to_player_.clear();
        token_to_player_.clear();
        dog_to_player_.clear();
        next_id_ = 0;
    }

private:
    TokenGenerator token_generator_;
    std::unordered_map<std::string, Player*> token_to_player_;
    std::unordered_map<std::pair<model::Dog::Id, MapId>, Player*, PairHasher> dog_to_player_;
    std::unordered_map<PlayerId, std::unique_ptr<Player>> id_to_player_;
    PlayerId next_id_ = 0;
};

} // namespace app
