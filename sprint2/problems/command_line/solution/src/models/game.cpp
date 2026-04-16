#include "game.h"

namespace model {

using namespace std::literals;
void Game::AddMap(Map map) {
  const size_t index = maps_.size();
  auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index);
  if (!inserted) {
    throw std::invalid_argument("Map with id "s + *map.GetId() +
                                " already exists"s);
  }
  try {
    maps_.emplace_back(std::move(map));
  } catch (const std::bad_alloc &ex) {
    map_id_to_index_.erase(it);
    throw;
  }
}

const Game::Maps &Game::GetMaps() const noexcept { return maps_; }

const Map *Game::FindMap(const Map::Id &id) const noexcept {
  if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
    return &maps_.at(it->second);
  }
  return nullptr;
}

std::shared_ptr<GameSession> Game::CreateNewSession(const Map *map_ptr) {
  const auto [it, _] = sessions_.emplace(
      *map_ptr->GetId(),
      std::make_shared<GameSession>(map_ptr, ioc_, tick_period_,
                                    randomize_spawn_point_));

  it->second->StartUpdater();
  return it->second;
};

std::shared_ptr<GameSession> Game::GetSession(const Map *map_ptr) {
  auto it = sessions_.find(*map_ptr->GetId());
  if (it == sessions_.end()) {
    return CreateNewSession(map_ptr);
  }
  return it->second;
};

std::shared_ptr<GameSession>
Game::GetSessionByToken(std::string_view token) const {
  const auto it = sessions_by_token_.find(token);
  if (it != sessions_by_token_.end()) {
    return it->second;
  }

  return nullptr;
};

const Player *Game::AddPlayer(const std::string &user_name,
                              const Map *map_ptr) {
  auto session_ptr = GetSession(map_ptr);
  const auto player_ptr = session_ptr->GetPlayerByName(user_name);
  if (player_ptr) {
    return player_ptr;
  }

  auto dog = std::make_shared<Dog>(user_name, map_ptr);
  const auto new_player_ptr = session_ptr->AddPlayer(dog, randomizer_());

  sessions_by_token_.emplace(new_player_ptr->GetToken(), session_ptr);
  return new_player_ptr;
}

const Player *Game::GetPlayerByToken(std::string_view token) const {
  return GetSessionByToken(token)->GetPlayerByToken(token);
};

const Game::Sessions &Game::GetSessions() const { return sessions_; };
Game::Sessions &Game::GetSessions() { return sessions_; };

void Game::UpdateState(const std::chrono::steady_clock::duration &duration) {
  for (auto &session : GetSessions()) {
    session.second->UpdateState(duration);
  }
}
const bool Game::IsRandomSpawnStrategy() const {
  return randomize_spawn_point_;
}
const boost::optional<uint64_t> Game::GetTickPeriod() const {
  return tick_period_;
}
const services::Authorization &Game::GetAuthorization() const {
  return authorization_;
};
} // namespace model