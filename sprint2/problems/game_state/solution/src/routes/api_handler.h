#pragma once

#include "game.h"
#include "iroute.h"
#include "log_response_data.h"

namespace Routes {

class ApiHandler : public IRoute {
public:
  explicit ApiHandler(model::Game &game);

  ApiHandler(const ApiHandler &) = delete;
  ApiHandler &operator=(const ApiHandler &) = delete;

  http_handler::LogResponseData operator()(const It begin, const It end,
                                           const Method method,
                                           const Request &req) const override;
};
} // namespace Routes