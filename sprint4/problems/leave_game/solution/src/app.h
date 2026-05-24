#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <boost/json/array.hpp>
#include "model.h"
#include "players.h"
#include <boost/signals2.hpp>
#include "postgres.h"


namespace app {

class InvalidArgument : public std::exception {
private:
    std::string message;
public:
    explicit InvalidArgument(const std::string& msg)
        : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class Application {
public:
    Application(model::Game& game, Players& players,postgres::Database* db = nullptr);

    struct JoinGameResult {
        std::string token;
        Player::Id player_id;
    };
    JoinGameResult JoinGame(const std::string& user_name, const std::string& map_id_str);

    std::vector<std::pair<Player::Id, std::string>> GetPlayers(const std::string& token);

    struct PlayerState {
        Player::Id player_id;
        double x, y;
        double vx, vy;
        model::Direction dir;
        std::vector<model::Loot> bag;
        int score;
    };
    std::vector<PlayerState> GetGameState(const std::string& token);

    const model::Game::Maps& GetMaps() const;
    const model::Map* FindMap(const std::string& map_id_str) const;

    void SetPlayerSpeed(const std::string& token, std::string str_dir);

    const std::vector<model::Loot>& GetLoot(const std::string& token) const;

    static boost::json::array GetLootTypesForMap(const model::Map::Id& map_id);

    using TickSignal = boost::signals2::signal<void(std::chrono::milliseconds delta)>;
    [[nodiscard]] boost::signals2::connection DoOnTick(const TickSignal::slot_type& handler) {
        return tick_signal_.connect(handler);
    }

    void UpdateGameState(std::chrono::milliseconds tick_time) {
        game_.UpdateState(tick_time);

        // Обрабатываем удалённых собак
        for (auto* session : game_.GetSessions()) {
            std::vector<Player*> players_to_remove;
            std::transform(session->GetDogs().begin(), session->GetDogs().end(),
                           std::back_inserter(players_to_remove),
                           [&](const auto& dog) -> Player* {
                               if (!dog->IsDeleted()) return nullptr; // если собака жива не трогаем player
                               auto* player = players_.FindByDogIdAndMapId(dog->GetId(), *session->GetMap().GetId()); // находим player
                               if (player) {
                                   SaveRecordToDB(dog->GetName(), dog->GetScore(), dog->GetActiveTime()); // сохраняем в БД
                                   return player; // добавляем player в вектор players_to_remove
                               }
                               return nullptr;  // если не нашли игрока по собаке
                           });
            auto end_it = std::remove(players_to_remove.begin(), players_to_remove.end(), nullptr); // получаем it на новый конец валидных значений
            players_to_remove.erase(end_it,players_to_remove.end());

            // Удаляем игроков
            for (auto* player : players_to_remove) {
                players_.RemovePlayer(player->GetId());
            }
            session->RemoveDeletedDogs();
        }

        tick_signal_(tick_time);
    }


    model::Game& GetGame() {
        return game_;
    }
    const Players& GetPlayers() const { return players_; }

    std::vector<postgres::PlayerRecord> GetRecords(size_t start, size_t maxItems) const {
        return db_->GetRecords(start,maxItems);
    }

    void SaveRecordToDB(const std::string& name, int score, double play_time) {db_->SaveRecord(name,score,play_time);}


private:
    model::Game& game_;
    Players& players_;
    TickSignal tick_signal_;
    postgres::Database* db_;
};

} // namespace app
