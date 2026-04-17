#include "game_session.h"
#include "dog.h"
#include "players_handler.h"
#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <memory>

namespace model {

const std::deque<Player> &GameSession::GetPlayers() const {
  return players_handler_.GetAll();
}

const PlayersHandler &GameSession::GetPlayersHandler() const {
  return players_handler_;
};

const Player *GameSession::AddPlayer(std::shared_ptr<Dog> dog,
                                     std::string token) {
  return players_handler_.AddPlayer(std::move(dog), std::move(token),
                                    weak_from_this());
}

const Player *GameSession::GetPlayerByToken(std::string_view token) const {
  return players_handler_.FindByToken(token);
}

const Player *GameSession::GetPlayerByName(std::string_view name) const {
  return players_handler_.FindByName(name);
}

const Map *GameSession::GetMap() const { return map_; };

void GameSession::UpdateState(
    const std::chrono::steady_clock::duration &duration) {
  for (const auto &player : GetPlayers()) {
    boost::asio::dispatch(strand_, [&player, duration]() {
      player.GetCurrentDog()->Move(duration);
    });
  };
}

const GameSession::Strand &GameSession::GetStrand() const { return strand_; };

GameSession::GameSession(const Map *map, boost::asio::io_context &ioc,
                         const boost::optional<uint64_t> &tick_period,
                         bool randomize_spawn_point)
    : map_{map}, strand_{boost::asio::make_strand(ioc)},
      tick_period_{tick_period}, randomize_spawn_point_{randomize_spawn_point} {
}
const void GameSession::StartUpdater() {
  if (!tick_period_ || ticker_) {
    return;
  }
  if (tick_period_) {
    auto weak_this = weak_from_this();
    ticker_ = std::make_shared<Ticker>(
        strand_, std::chrono::milliseconds{*tick_period_},
        [weak_this](std::chrono::milliseconds delta) {
          if (auto shared_this = weak_this.lock()) {
            shared_this->UpdateState(delta);
          }
        });
    ticker_->Start();
  }
};
} // namespace model