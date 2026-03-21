#pragma once

#ifdef WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio.hpp>

#include "Hamburger.h"

namespace net = boost::asio;
namespace sys = boost::system;
namespace ph = std::placeholders;
using namespace std::chrono;
using namespace std::literals;
using Timer = net::steady_timer;

// Функция, которая будет вызвана по окончании обработки заказа
using OrderHandler = std::function<void(sys::error_code ec, int id, Hamburger* hamburger)>;

class Restaurant {
public:
    explicit Restaurant(net::io_context& io);
    int MakeHamburger(bool with_onion, OrderHandler handler);

private:
    net::io_context& io_;
    int next_order_id_ = 0;
};