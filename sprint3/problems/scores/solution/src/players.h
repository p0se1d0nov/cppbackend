#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include "player.h"
#include "token_generator.h"

namespace app {

class Players {
public:
    using PlayerId = Player::Id;

    PlayerId AddPlayer(std::string token, model::Dog& dog, model::GameSession& session) {
        PlayerId id = next_id_++;
        auto player = std::make_unique<Player>(id, std::move(token), dog, session);
        token_to_player_[player->GetToken()] = player.get();
        dog_to_player_[dog.GetId()] = player.get();
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

    Player* FindByDogId(model::Dog::Id dog_id) {
        auto it = dog_to_player_.find(dog_id);
        return it != dog_to_player_.end() ? it->second : nullptr;
    }

    std::string GenerateToken() {
        return token_generator_.Generate();
    }

private:
    TokenGenerator token_generator_;
    std::unordered_map<std::string, Player*> token_to_player_;
    std::unordered_map<model::Dog::Id, Player*> dog_to_player_;
    std::unordered_map<PlayerId, std::unique_ptr<Player>> id_to_player_;
    PlayerId next_id_ = 0;
};

} // namespace app
