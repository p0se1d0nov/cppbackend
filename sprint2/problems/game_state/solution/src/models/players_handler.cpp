
#include "players_handler.h"
#include "dog.h"
#include "player.h"

#include <deque>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace model {

const Player *PlayersHandler::AddPlayer(std::shared_ptr<Dog> dog,
                                        std::string token,
                                        std::weak_ptr<GameSession> session) {
  size_t index = players_.size();
  const auto &player = AddPlayerInner(std::move(dog), std::move(token),
                                      std::move(session), index);

  return &player;
}

const Player *
PlayersHandler::FindByDogIdAndMapId(std::string_view dog_id,
                                    std::string_view map_id) const {
  return nullptr;
}

const Player *PlayersHandler::FindByToken(std::string_view token) const {
  const auto it = players_by_token_.find(token);
  if (it == players_by_token_.end()) {
    return nullptr;
  }
  return it->second;
}

const Player *PlayersHandler::FindByName(std::string_view name) const {
  const auto it = players_by_name_.find(name);
  if (it == players_by_name_.end()) {
    return nullptr;
  }
  return it->second;
};

const std::deque<Player> &PlayersHandler::GetAll() const { return players_; };

size_t PlayersHandler::Size() const { return players_.size(); }

const Player &PlayersHandler::AddPlayerInner(std::shared_ptr<Dog> dog,
                                             std::string token,
                                             std::weak_ptr<GameSession> session,
                                             size_t id_in_session) {
  const auto &player = players_.emplace_back(std::move(dog), std::move(token),
                                             std::move(session), id_in_session);
  players_by_token_.emplace(player.GetToken(), &player);
  players_by_name_.emplace(player.GetName(), &player);
  return player;
}

} // namespace model
