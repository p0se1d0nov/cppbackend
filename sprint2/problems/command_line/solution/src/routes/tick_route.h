#pragma once

#include "game.h"
#include "iroute.h"

namespace Routes {

class TickRoute : public IRoute {

public:
  explicit TickRoute(model::Game &game);

  TickRoute(const TickRoute &) = delete;
  TickRoute &operator=(const TickRoute &) = delete;
  http_handler::LogResponseData operator()(It begin, It end, Method method,
                                           const Request &req) const override;
};
} // namespace Routes