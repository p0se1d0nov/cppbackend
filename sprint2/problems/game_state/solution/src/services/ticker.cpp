#include "ticker.h"

void Ticker::Start() {
  last_tick_ = Clock::now();
  boost::asio::dispatch(strand_,
                        [self = shared_from_this()] { self->ScheduleTick(); });
}
void Ticker::ScheduleTick() {
  assert(strand_.running_in_this_thread());
  timer_.expires_after(period_);
  timer_.async_wait([self = shared_from_this()](boost::system::error_code ec) {
    self->OnTick(ec);
  });
}
void Ticker::OnTick(boost::system::error_code ec) {
  using namespace std::chrono;
  assert(strand_.running_in_this_thread());

  if (!ec) {
    auto this_tick = Clock::now();
    auto delta = duration_cast<milliseconds>(this_tick - last_tick_);
    last_tick_ = this_tick;
    try {
      handler_(delta);
    } catch (...) {
    }
    ScheduleTick();
  }
}
