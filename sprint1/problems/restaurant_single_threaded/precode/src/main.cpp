#ifdef WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <syncstream>
#include <unordered_map>
#include <memory>
#include <thread>

#include "Restaurant.h"
#include "Logger.h"
#include "Hamburger.h"


namespace net = boost::asio;
namespace sys = boost::system;
namespace ph = std::placeholders;
using namespace std::chrono;
using namespace std::literals;
using Timer = net::steady_timer;


template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
} 


int main() {
    net::io_context io;

    Restaurant restaurant{io};

    Logger logger{"main"s};

    struct OrderResult {
        sys::error_code ec;
        Hamburger hamburger;
    };

    std::unordered_map<int, OrderResult> orders;
    auto handle_result = [&orders](sys::error_code ec, int id, Hamburger* h) {
        orders.emplace(id, OrderResult{ec, ec ? Hamburger{} : *h});
    };

    const int id1 = restaurant.MakeHamburger(false, handle_result);
    const int id2 = restaurant.MakeHamburger(true, handle_result);

    // До вызова io.run() никакие заказы не выполняются
    assert(orders.empty());

    const int num_workers = 4;
    RunWorkers(num_workers, [&io]{
        io.run();
    });

    // После вызова io.run() все заказы быть выполнены
    assert(orders.size() == 2u);
    {
        // Проверяем заказ без лука
        const auto& o = orders.at(id1);
        assert(!o.ec);
        assert(o.hamburger.IsCutletRoasted());
        assert(o.hamburger.IsPacked());
        assert(!o.hamburger.HasOnion());
    }
    {
        // Проверяем заказ с луком
        const auto& o = orders.at(id2);
        assert(!o.ec);
        assert(o.hamburger.IsCutletRoasted());
        assert(o.hamburger.IsPacked());
        assert(o.hamburger.HasOnion());
    }
}

