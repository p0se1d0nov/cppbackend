#include "game.h"
#include "iroute.h"
#include "log_response_data.h"

#include <boost/json/array.hpp>

namespace Routes {

class GameStateRoute : public IRoute {

public:
  explicit GameStateRoute(model::Game &game) : IRoute(game) {};

  GameStateRoute(const GameStateRoute &) = delete;
  GameStateRoute &operator=(const GameStateRoute &) = delete;
  http_handler::LogResponseData operator()(It begin, It end, Method method,
                                           const Request &req) const override;
};
} // namespace Routes