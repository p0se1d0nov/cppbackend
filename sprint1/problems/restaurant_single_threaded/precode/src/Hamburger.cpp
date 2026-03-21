#include "Hamburger.h"

using namespace std::literals;


[[nodiscard]] bool Hamburger::IsCutletRoasted() const
{
    return cutlet_roasted_;
}

void Hamburger::SetCutletRoasted()
{
    if (IsCutletRoasted())
    { // Котлету можно жарить только один раз
        throw std::logic_error("Cutlet has been roasted already"s);
    }
    cutlet_roasted_ = true;
}

[[nodiscard]] bool Hamburger::HasOnion() const
{
    return has_onion_;
}
// Добавляем лук
void Hamburger::AddOnion()
{
    if (IsPacked())
    { // Если гамбургер упакован, класть лук в него нельзя
        throw std::logic_error("Hamburger has been packed already"s);
    }
    AssureCutletRoasted(); // Лук разрешается класть лишь после прожаривания котлеты
    has_onion_ = true;
}

[[nodiscard]] bool Hamburger::IsPacked() const
{
    return is_packed_;
}

void Hamburger::Pack()
{
    AssureCutletRoasted(); // Нельзя упаковывать гамбургер, если котлета не прожарена
    is_packed_ = true;
}

// Убеждаемся, что котлета прожарена
void Hamburger::AssureCutletRoasted() const
{
    if (!cutlet_roasted_)
    {
        throw std::logic_error("Bread has not been roasted yet"s);
    }
}

std::ostream &operator<<(std::ostream &os, const Hamburger &h)
{
    return os << "Hamburger: "sv << (h.IsCutletRoasted() ? "roasted cutlet"sv : " raw cutlet"sv)
              << (h.HasOnion() ? ", onion"sv : ""sv)
              << (h.IsPacked() ? ", packed"sv : ", not packed"sv);
}