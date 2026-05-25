#pragma once
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include "model.h"
#include "app.h"

namespace geom {
template <typename Archive>
void serialize(Archive& ar, Point2D& point, [[maybe_unused]] const unsigned version) {
    ar & point.x;
    ar & point.y;
}

template <typename Archive>
void serialize(Archive& ar, Vec2D& vec, [[maybe_unused]] const unsigned version) {
    ar & vec.x;
    ar & vec.y;
}
}

namespace model {
template <typename Archive>
void serialize(Archive& ar, Loot& loot, [[maybe_unused]] const unsigned version) {
    ar & loot.id;
    ar & loot.type;
    ar & loot.x;
    ar & loot.y;
    ar & loot.value;
    ar & loot.on_map;
}

template <typename Archive>
void serialize(Archive& ar, Direction& dir, [[maybe_unused]] const unsigned version) {
    int temp = static_cast<int>(dir);
    ar & temp;
    dir = static_cast<Direction>(temp);
}
}

namespace serialization {

class DogRepr {
public:
    DogRepr() = default;
    explicit DogRepr(const model::Dog& dog)
        : id_(dog.GetId())
        , name_(dog.GetName())
        , x_(dog.GetX()), y_(dog.GetY())
        , vx_(dog.GetVx()), vy_(dog.GetVy())
        , dir_(dog.GetDirection())
        , score_(dog.GetScore())
        , bag_(dog.GetBag())
    {}

    model::Dog Restore() const {
        model::Dog dog{id_, name_};
        dog.SetPosition(x_, y_);
        dog.SetSpeed(vx_, vy_);
        dog.SetDirection(dir_);
        for (const auto& loot : bag_) {
            dog.AddToBag(loot);
        }
        return dog;
    }

    model::Dog::Id GetId() const {
        return id_;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & id_;
        ar & name_;
        ar & x_;
        ar & y_;
        ar & vx_;
        ar & vy_;
        ar & dir_;
        ar & score_;
        ar & bag_;
    }

private:
    model::Dog::Id id_ = 0;
    std::string name_;
    double x_ = 0.0, y_ = 0.0;
    double vx_ = 0.0, vy_ = 0.0;
    model::Direction dir_ = model::Direction::NORTH;
    int score_ = 0;
    std::vector<model::Loot> bag_;
};

class GameSessionRepr {
public:
    GameSessionRepr() = default;
    explicit GameSessionRepr(const model::GameSession& session)
        : map_id_(*session.GetMap().GetId())
        , next_dog_id_(session.GetNextDogId())
        , next_loot_id_(session.GetNextLootId())
    {
        for (const auto& dog : session.GetDogs()) {
            dogs_.emplace_back(*dog);
        }
        for (const auto& loot : session.GetLoot()) {
            loots_.push_back(loot);
        }
    }

    void RestoreTo(model::GameSession& session) const {
        for (const auto& dog_repr : dogs_) {
            auto dog = std::make_unique<model::Dog>(dog_repr.Restore());
            session.AddDog(std::move(dog));
        }
        for (const auto& loot : loots_) {
            session.AddLoot(loot);
        }
        session.SetNextDogId(next_dog_id_);
        session.SetNextLootId(next_loot_id_);
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & map_id_;
        ar & dogs_;
        ar & loots_;
        ar & next_dog_id_;
        ar & next_loot_id_;
    }

    const std::string& GetMapId() const { return map_id_; }

private:
    std::string map_id_;
    std::vector<DogRepr> dogs_;
    std::vector<model::Loot> loots_;
    uint64_t next_dog_id_;
    uint64_t next_loot_id_;
};

class PlayerRepr {
public:
    PlayerRepr() = default;
    explicit PlayerRepr(const app::Player& player)
        : id_(player.GetId())
        , token_(player.GetToken())
        , dog_id_(player.GetDog().GetId())
        , map_id_(*player.GetSession().GetMap().GetId())
    {}

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & id_;
        ar & token_;
        ar & dog_id_;
        ar & map_id_;
    }

    uint64_t GetId() const { return id_; }
    std::string GetMapId() const { return map_id_; }
    std::string GetToken() const { return token_; }
    model::Dog::Id GetDogId() const { return dog_id_; }

private:
    uint64_t id_;
    std::string token_;
    model::Dog::Id dog_id_;
    std::string map_id_;
};

class GameStateRepr {
public:
    GameStateRepr() = default;
    explicit GameStateRepr(const model::Game& game, const app::Players& players) {
        for (const auto* session : game.GetSessions()) {
            sessions_.emplace_back(*session);
        }
        for (const auto* player : players.GetAllPlayers()) {
            players_.emplace_back(*player);
        }
    }

    void RestoreTo(model::Game& game, app::Players& players) const {
        for (const auto& sess_repr : sessions_) {
            auto& session = game.GetOrCreateSession(model::Map::Id(sess_repr.GetMapId()));
            session.ClearState();
            sess_repr.RestoreTo(session);
        }

        players.Clear();

        for (const auto& player_repr : players_) {
            auto* session = game.FindSession(model::Map::Id(player_repr.GetMapId()));
            if (!session) {
                throw std::runtime_error("Session not found for map " + player_repr.GetMapId());
            }
            model::Dog* dog = nullptr;
            for (const auto& d : session->GetDogs()) {
                if (d->GetId() == player_repr.GetDogId()) {
                    dog = d.get();
                    break;
                }
            }
            if (!dog) {
                throw std::runtime_error("Dog not found with id " + std::to_string(player_repr.GetDogId()));
            }
            players.RestorePlayer(player_repr.GetId(), player_repr.GetToken(), *dog, *session);
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & sessions_;
        ar & players_;
    }

private:
    std::vector<GameSessionRepr> sessions_;
    std::vector<PlayerRepr> players_;
};

} // namespace serialization
