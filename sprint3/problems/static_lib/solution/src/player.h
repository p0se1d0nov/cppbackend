#pragma once
#include <cstdint>
#include <string>
#include "model.h"

using namespace model;

namespace app {

class Player {
public:
    using Id = uint64_t;

    Player(Id id, std::string token, Dog& dog, GameSession& session)
        : id_(id), token_(std::move(token)), dog_(&dog), session_(&session) {}

    Id GetId() const { return id_; }
    const std::string& GetToken() const { return token_; }
    Dog& GetDog() const { return *dog_; }
    GameSession& GetSession() const { return *session_; }

private:
    Id id_;
    std::string token_;
    Dog* dog_;
    GameSession* session_;
};

} // namespace app
