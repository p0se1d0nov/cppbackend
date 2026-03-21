#include "Order.h"


using namespace std::literals;
using namespace std::chrono;

Order::Order(net::io_context &io, int id, bool with_onion, OrderHandler handler)
    : io_{io}, id_{id}, with_onion_{with_onion}, handler_{std::move(handler)}
{}

// Запускает асинхронное выполнение заказа
void Order::Execute()
{
    logger_.LogMessage("Order has been started."sv);
    RoastCutlet();
    if (with_onion_)
    {
        MarinadeOnion();
    }
}

void Order::RoastCutlet() {
    logger_.LogMessage("Start roasting cutlet"sv);
    roast_timer_.async_wait(
        // OnRoasted будет вызван последовательным исполнителем strand_
        net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
            self->OnRoasted(ec);
    }));
}

void Order::OnRoasted(sys::error_code ec)
{
    ThreadChecker checker{counter_};
    if (ec)
    {
        logger_.LogMessage("Roast error : "s + ec.what());
    }
    else
    {
        logger_.LogMessage("Cutlet has been roasted."sv);
        hamburger_.SetCutletRoasted();
    }
    CheckReadiness(ec);
}

void Order::MarinadeOnion()
{
    logger_.LogMessage("Start marinading onion"sv);
    marinade_timer_.async_wait([self = shared_from_this()](sys::error_code ec)
    { self->OnOnionMarinaded(ec); });
}

void Order::OnOnionMarinaded(sys::error_code ec)
{
    logger_.LogMessage("Start marinading onion"sv);
    marinade_timer_.async_wait(
        // OnOnionMarinaded будет вызван последовательным исполнителем strand_
        net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
            self->OnOnionMarinaded(ec);
    }));
}

void Order::CheckReadiness(sys::error_code ec)
{
    if (delivered_)
    {
        // Выходим, если заказ уже доставлен либо клиента уведомили об ошибке
        return;
    }
    if (ec)
    {
        // В случае ошибки уведомляем клиента о невозможности выполнить заказ
        return Deliver(ec);
    }

    // Самое время добавить лук
    if (CanAddOnion())
    {
        logger_.LogMessage("Add onion"sv);
        hamburger_.AddOnion();
    }

    // Если все компоненты гамбургера готовы, упаковываем его
    if (IsReadyToPack())
    {
        Pack();
    }
}

void Order::Deliver(sys::error_code ec)
{
    // Защита заказа от повторной доставки
    delivered_ = true;
    // Доставляем гамбургер в случае успеха либо nullptr, если возникла ошибка
    handler_(ec, id_, ec ? nullptr : &hamburger_);
}

[[nodiscard]] bool Order::CanAddOnion() const
{
    // Лук можно добавить, если котлета обжарена, лук замаринован, но пока не добавлен
    return hamburger_.IsCutletRoasted() && onion_marinaded_ && !hamburger_.HasOnion();
}

[[nodiscard]] bool Order::IsReadyToPack() const
{
    // Если котлета обжарена и лук добавлен, как просили, гамбургер можно упаковывать
    return hamburger_.IsCutletRoasted() && (!with_onion_ || hamburger_.HasOnion());
}

void Order::Pack() {
    logger_.LogMessage("Packing"sv);

    // Просто потребляем ресурсы процессора в течение 0,5 с.
    auto start = steady_clock::now();
    while (steady_clock::now() - start < 500ms) {
    }

    hamburger_.Pack();
    logger_.LogMessage("Packed"sv);

    Deliver({});
}