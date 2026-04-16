#pragma once

#include "boost_beast.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <chrono>

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
  using Strand = boost::asio::strand<boost::asio::io_context::executor_type>;
  using Handler = std::function<void(std::chrono::milliseconds delta)>;

  Ticker(Strand strand, std::chrono::milliseconds period, Handler handler)
      : strand_{strand}, period_{period}, handler_{std::move(handler)} {}

  void Start();

private:
  void ScheduleTick();

  void OnTick(boost::system::error_code ec);

  using Clock = std::chrono::steady_clock;

  Strand strand_;
  std::chrono::milliseconds period_;
  boost::asio::steady_timer timer_{strand_};
  Handler handler_;
  Clock::time_point last_tick_;
};