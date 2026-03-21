#include "Restaurant.h"
#include "Order.h"
#include <memory>

Restaurant::Restaurant(net::io_context& io)
    : io_(io) {
}

int Restaurant::MakeHamburger(bool with_onion, OrderHandler handler) {
    const int order_id = ++next_order_id_;
    std::make_shared<Order>(io_, order_id, with_onion, std::move(handler))->Execute();
    return order_id;
}
