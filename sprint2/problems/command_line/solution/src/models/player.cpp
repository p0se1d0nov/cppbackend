#include "player.h"
#include "dog.h"
#include "game_session.h"

#include <memory>
#include <string>
#include <utility>

namespace model {

Player::Player(std::shared_ptr<Dog> dog, std::string token,
               std::weak_ptr<GameSession> session, size_t id_in_session)
    : dog_{dog}, token_{std::move(token)}, session_{session},
      id_in_session_{id_in_session} {}

Player::Player(Player &&player) noexcept
    : dog_(std::move(player.dog_)), token_(std::move(player.token_)),
      session_(std::move(player.session_)),
      id_in_session_(player.id_in_session_) {
  id_in_session_ = player.id_in_session_;
}

Player &Player::operator=(Player &&player) noexcept {
  if (&player == this) {
    return *this;
  }
  std::swap(dog_, player.dog_);
  std::swap(token_, player.token_);
  std::swap(session_, player.session_);
  id_in_session_ = player.id_in_session_;
  return *this;
}

const std::string &Player::GetToken() const { return token_; }

const std::string &Player::GetName() const { return dog_->GetId(); }

size_t Player::GetSessionId() const { return id_in_session_; }

std::weak_ptr<GameSession> Player::GetSessionPtr() const { return session_; }

const std::shared_ptr<Dog> Player::GetCurrentDog() const { return dog_; }

} // namespace model