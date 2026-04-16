#pragma once
#include "game.h"
#include "iroute.h"
#include "log_response_data.h"

namespace Routes {

class ApiVersionsRoute : public IRoute {
public:
  explicit ApiVersionsRoute(model::Game &game);

  ApiVersionsRoute(const ApiVersionsRoute &) = delete;
  ApiVersionsRoute &operator=(const ApiVersionsRoute &) = delete;

  http_handler::LogResponseData operator()(const It begin, const It end,
                                           const Method method,
                                           const Request &req) const override;
};
} // namespace Routes