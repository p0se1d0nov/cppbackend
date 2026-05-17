#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <boost/json/array.hpp>
#include "model.h"
#include "players.h"

namespace app {

class InvalidArgument : public std::exception {
private:
    std::string message;
public:
    InvalidArgument(const std::string& msg)
        : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class Application {
public:
    Application(model::Game& game, Players& players);

    // Сценарий входа в игру
    struct JoinGameResult {
        std::string token;
        Player::Id player_id;
    };
    JoinGameResult JoinGame(const std::string& user_name, const std::string& map_id_str);

    // Сценарий получения списка игроков (имена)
    std::vector<std::pair<Player::Id, std::string>> GetPlayers(const std::string& token);

    // Сценарий получения состояния игры
    struct PlayerState {
        Player::Id player_id;
        double x, y;
        double vx, vy;
        model::Direction dir;
    };
    std::vector<PlayerState> GetGameState(const std::string& token);

    // Методы для работы с картами
    const model::Game::Maps& GetMaps() const;
    const model::Map* FindMap(const std::string& map_id_str) const;

    void SetPlayerSpeed(const std::string& token, std::string str_dir);

    void UpdateGameState(std::chrono::milliseconds tick_time) {
        game_.UpdateState(tick_time);
    }

    const std::vector<model::Loot>& GetLoot(const std::string& token) const;

    boost::json::array GetLootTypesForMap(const model::Map::Id& map_id) const;

private:
    model::Game& game_;
    Players& players_;
};

} // namespace app
