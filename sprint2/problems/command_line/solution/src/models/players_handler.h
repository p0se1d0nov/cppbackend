#pragma once
#include "dog.h"
#include "player.h"

#include <deque>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace model {
class GameSession;
class PlayersHandler {
public:
  PlayersHandler() = default;

public:
  const Player *AddPlayer(std::shared_ptr<Dog> dog, std::string token,
                          std::weak_ptr<GameSession> session);

  const Player *FindByDogIdAndMapId(std::string_view dog_id,
                                    std::string_view map_id) const;

  const Player *FindByToken(std::string_view token) const;

  const Player *FindByName(std::string_view) const;

  const std::deque<Player> &GetAll() const;

  size_t Size() const;

private:
  const Player &AddPlayerInner(std::shared_ptr<Dog> dog, std::string token,
                               std::weak_ptr<GameSession> session,
                               size_t id_in_session);

private:
  std::deque<Player> players_;
  std::unordered_map<std::string_view, const Player *> players_by_token_;
  std::unordered_map<std::string_view, const Player *> players_by_name_;
};
} // namespace model
