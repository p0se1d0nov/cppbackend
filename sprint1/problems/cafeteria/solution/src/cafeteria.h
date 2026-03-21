#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;
namespace sys = boost::system;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria
{
public:
    explicit Cafeteria(net::io_context &io)
        : io_{io}
    {
    }

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler)
    {
        auto sausage = store_.GetSausage();
        auto bread = store_.GetBread();

        struct CookState
        {
            std::shared_ptr<Sausage> sausage;
            std::shared_ptr<Bread> bread;
            HotDogHandler handler;
            net::io_context &io;
            std::atomic<int> ready_count = 0;
            int hot_dog_id;

            void on_ready()
            {
                if (++ready_count == 2)
                {
                    try
                    {
                        HotDog hot_dog{hot_dog_id, sausage, bread};
                        net::post(io, [handler = std::move(handler), hot_dog = std::move(hot_dog)]() mutable
                                  { handler(std::move(hot_dog)); });
                    }
                    catch (...)
                    {
                        net::post(io, [handler = std::move(handler)]() mutable
                                  { handler(Result<HotDog>::FromCurrentException()); });
                    }
                }
            }
        };

        auto state = std::shared_ptr<CookState>(new CookState{sausage, bread, std::move(handler), std::ref(io_), 0, next_hot_dog_id_++});

        // Для сосиски: жарить 1.5 секунды
        sausage->StartFry(*gas_cooker_, [state]()
                          {
            auto timer = std::make_shared<net::steady_timer>(state->io, std::chrono::milliseconds(1500));
            timer->async_wait([state, timer](const sys::error_code& ec) {
                if (!ec) {
                    state->sausage->StopFry();
                    state->on_ready();
                }
            }); });

        // Для булки: печь 1 секунду
        bread->StartBake(*gas_cooker_, [state]()
                         {
            auto timer = std::make_shared<net::steady_timer>(state->io, std::chrono::milliseconds(1000));
            timer->async_wait([state, timer](const sys::error_code& ec) {
                if (!ec) {
                    state->bread->StopBaking();
                    state->on_ready();
                }
            }); });
    }

private:
    net::io_context &io_;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
    int next_hot_dog_id_ = 0;
};
