#pragma once

#include <string_view>

namespace http_endpoints {

class HttpEndpoints {
public:
    std::string_view ApiPrefix() {
        return api_prefix_;
    }
    std::string_view ApiV1Prefix() {
        return api_v1_prefix_;
    }
    std::string_view Maps() {
        return maps_;
    }
    std::string_view MapById () {
        return map_by_id_;
    }
    std::string_view GameJoin () {
        return game_join_;
    }
    std::string_view GamePlayers() {
        return game_players_;
    }
    std::string_view GameState () {
        return game_state_;
    }
    std::string_view GamePlayerAction () {
        return game_player_action_;
    }
    std::string_view GameTick () {
        return game_tick_;
    }
    std::string_view GameRecords () {
        return game_records_;
    }

private:
    const std::string_view api_prefix_ = "/api/";
    const std::string_view api_v1_prefix_ = "/api/v1";
    const std::string_view maps_ = "/maps";
    const std::string_view map_by_id_ = "/maps/";
    const std::string_view game_join_ = "/game/join";
    const std::string_view game_players_ = "/game/players";
    const std::string_view game_state_ = "/game/state";
    const std::string_view game_player_action_ = "/game/player/action";
    const std::string_view game_tick_ = "/game/tick";
    const std::string_view game_records_ = "/game/records";
};

}
