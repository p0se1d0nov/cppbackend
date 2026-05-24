#pragma once
#include <cstdint>
#include <string>
#include "model.h"

namespace app {

class Player {
public:
    using Id = uint64_t;

    Player(Id id, std::string token, model::Dog& dog, model::GameSession& session)
        : id_(id), token_(std::move(token)), dog_(&dog), session_(&session) {}

    Id GetId() const { return id_; }
    const std::string& GetToken() const { return token_; }
    model::Dog& GetDog() const { return *dog_; }
    model::GameSession& GetSession() const { return *session_; }

private:
    Id id_;
    std::string token_;
    model::Dog* dog_;
    model::GameSession* session_;
};

} // namespace app
