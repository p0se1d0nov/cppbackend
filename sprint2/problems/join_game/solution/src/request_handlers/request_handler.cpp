
#include "request_handler.h"

#include "game.h"

namespace http_handler {

RequestHandler::RequestHandler(model::Game &game, std::string static_catalog)
    : game_{game}, api_worker_{game_},
      static_catalog_{std::move(static_catalog)} {}

RequestHandler::RequestHandler(RequestHandler &&handler) noexcept
    : game_{handler.game_}, api_worker_{game_},
      static_catalog_{std::move(handler.static_catalog_)} {};

RequestHandler &RequestHandler::operator=(RequestHandler &&handler) noexcept {
  if (&handler == this) {
    return *this;
  }

  std::swap(game_, handler.game_);
  std::swap(static_catalog_, handler.static_catalog_);

  return *this;
};
} // namespace http_handler