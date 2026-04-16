#pragma once

#include "authorization.h"
#include "game_session.h"
#include "map.h"
#include "randomizer.h"
#include "tagged.h"

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <deque>
#include <unordered_map>
#include <utility>

namespace model {
class Game {
public:
  using Maps = std::deque<Map>;
  using Sessions =
      std::unordered_map<std::string_view, std::shared_ptr<GameSession>>;

  Game(boost::asio::io_context &ioc, boost::optional<uint64_t> tick_period,
       bool randomize_spawn_point)
      : ioc_{ioc}, tick_period_{tick_period},
        randomize_spawn_point_{randomize_spawn_point} {}
  Game(const Game &) = delete;
  Game(Game &&game)
      : ioc_{game.ioc_}, maps_{std::move(game.maps_)},
        map_id_to_index_{std::move(game.map_id_to_index_)},
        sessions_{std::move(game.sessions_)} {};
  Game &operator=(const Game &) = delete;
  Game &operator=(Game &&game) {
    if (this == &game) {
      return *this;
    }
    Game tmp_game{std::move(game)};
    Swap(Game{std::move(game)});
    return *this;
  };

  void Swap(Game game) {
    std::swap(maps_, game.maps_);
    std::swap(map_id_to_index_, game.map_id_to_index_);
    std::swap(sessions_, game.sessions_);
  };

  void AddMap(Map map);
  const Maps &GetMaps() const noexcept;
  const Map *FindMap(const Map::Id &id) const noexcept;

  std::shared_ptr<GameSession> CreateNewSession(const Map *map);
  std::shared_ptr<GameSession> GetSession(const Map *map);
  std::shared_ptr<GameSession> GetSessionByToken(std::string_view token) const;
  const Player *GetPlayerByToken(std::string_view token) const;
  const boost::optional<uint64_t> GetTickPeriod() const;
  const bool IsRandomSpawnStrategy() const;
  const services::Authorization &GetAuthorization() const;

  const Player *AddPlayer(const std::string &user_name, const Map *map_ptr);

  void UpdateState(const std::chrono::steady_clock::duration &duration);

  const Sessions &GetSessions() const;
  Sessions &GetSessions();

private:
  using MapIdHasher = util::TaggedHasher<Map::Id>;
  using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

  boost::asio::io_context &ioc_;
  boost::optional<uint64_t> tick_period_;
  bool randomize_spawn_point_;
  Maps maps_;
  MapIdToIndex map_id_to_index_;
  Sessions sessions_;
  Sessions sessions_by_token_;
  services::Authorization authorization_;
  util::Randomizer randomizer_{};
};
} // namespace model