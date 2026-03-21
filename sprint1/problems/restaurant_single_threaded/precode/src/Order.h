#pragma once

#ifdef WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio.hpp>
#include <chrono>

#include "Logger.h"
#include "Hamburger.h"
#include "ThreadChecker.h"

namespace net = boost::asio;
namespace sys = boost::system;
using Timer = net::steady_timer;

using OrderHandler = std::function<void(sys::error_code ec, int id, Hamburger *hamburger)>;

class Order : public std::enable_shared_from_this<Order>
{
public:
    explicit Order(net::io_context &io, int id, bool with_onion, OrderHandler handler);
    // Запускает асинхронное выполнение заказа
    void Execute();

private:
    void RoastCutlet();
    void OnRoasted(sys::error_code ec);
    void MarinadeOnion();
    void OnOnionMarinaded(sys::error_code ec);
    void CheckReadiness(sys::error_code ec);
    void Deliver(sys::error_code ec);
    [[nodiscard]] bool CanAddOnion() const;
    [[nodiscard]] bool IsReadyToPack() const;
    void Pack();

    net::io_context &io_;
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    int id_;
    bool with_onion_;
    OrderHandler handler_;
    Logger logger_{std::to_string(id_)};
    Timer roast_timer_{io_, 1ms};
    Timer marinade_timer_{io_, 1ms};
    Hamburger hamburger_;
    bool onion_marinaded_ = false;
    bool delivered_ = false;
    std::atomic_int counter_{0};
};
