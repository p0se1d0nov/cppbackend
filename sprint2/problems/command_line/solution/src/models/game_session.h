#pragma once

#include "dog.h"
#include "players_handler.h"
#include "ticker.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/optional.hpp>
#include <deque>
#include <memory>

namespace model {

class Map;

class GameSession : public std::enable_shared_from_this<GameSession> {
  using Dogs = std::deque<std::shared_ptr<Dog>>;
  using Strand = boost::asio::strand<boost::asio::io_context::executor_type>;

public:
  GameSession(const Map *map, boost::asio::io_context &ioc,
              const boost::optional<uint64_t> &tick_period,
              bool randomize_spawn_point);

  GameSession(const GameSession &) = delete;
  GameSession &operator=(const GameSession &) = delete;

public:
  const std::deque<Player> &GetPlayers() const;
  const PlayersHandler &GetPlayersHandler() const;
  const Player *AddPlayer(std::shared_ptr<Dog> dog, std::string token);
  const Player *GetPlayerByToken(std::string_view token) const;
  const Player *GetPlayerByName(std::string_view name) const;
  const Map *GetMap() const;
  const Strand &GetStrand() const;
  const void StartUpdater();

  void UpdateState(const std::chrono::steady_clock::duration &duration);

private:
  const Map *map_;
  const Strand strand_;
  const boost::optional<uint64_t> tick_period_;
  bool randomize_spawn_point_;
  std::shared_ptr<Ticker> ticker_;
  PlayersHandler players_handler_;
};
} // namespace model