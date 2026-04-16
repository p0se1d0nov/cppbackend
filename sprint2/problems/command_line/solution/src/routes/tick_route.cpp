#include "tick_route.h"
#include "iroute.h"
#include "request_constant_values.h"
#include <chrono>

namespace Routes {
TickRoute::TickRoute(model::Game &game) : IRoute(game) {};

http_handler::LogResponseData TickRoute::operator()(const It begin,
                                                    const It end,
                                                    const Method method,
                                                    const Request &req) const {
  using namespace std::literals;

  if (req.method() != http::verb::post) {
    return method(http::status::method_not_allowed, INVALID_METHOD, "POST"sv);
  }

  const auto tick_period_opt = game_.GetTickPeriod();
  if (tick_period_opt) {
    return method(http::status::bad_request, BAD_REQUEST, ""sv);
  }

  boost::json::object json_obj;
  try {
    json_obj = boost::json::parse(req.body()).as_object();
  } catch (const std::exception &ex) {
    return method(http::status::bad_request, FAILED_TICK, ""sv);
  }

  const auto time_delta_ptr = json_obj.if_contains("timeDelta");

  if (!time_delta_ptr || !time_delta_ptr->is_int64()) {
    return method(http::status::bad_request, FAILED_TICK, ""sv);
  }

  const auto time_delta = time_delta_ptr->as_int64();

  if (time_delta <= 0) {
    return method(http::status::bad_request, FAILED_TICK, ""sv);
  }

  const auto duration = std::chrono::milliseconds(time_delta);
  this->game_.UpdateState(duration);

  return method(http::status::ok, EMPTY_BODY, ""sv);
}
} // namespace Routes