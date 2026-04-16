#pragma once
#include "dog.h"

#include <memory>
#include <string>

namespace model {

class GameSession;

class Player {
public:
  Player(std::shared_ptr<Dog> dog, std::string token,
         std::weak_ptr<GameSession> session, size_t id_in_session);

  Player(const Player &) = delete;
  Player &operator=(const Player &) = delete;

  Player(Player &&player) noexcept;

  Player &operator=(Player &&player) noexcept;

  const std::string &GetToken() const;

  const std::string &GetName() const;

  size_t GetSessionId() const;
  std::weak_ptr<GameSession> GetSessionPtr() const;

  const std::shared_ptr<Dog> GetCurrentDog() const;

protected:
  std::shared_ptr<Dog> dog_;
  std::string token_;
  std::weak_ptr<GameSession> session_;
  size_t id_in_session_;
};
} // namespace model