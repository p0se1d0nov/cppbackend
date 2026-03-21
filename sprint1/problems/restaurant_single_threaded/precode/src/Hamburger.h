#pragma once

#include <iostream>

using namespace std::literals;

class Hamburger
{
public:
    [[nodiscard]] bool IsCutletRoasted() const;
    void SetCutletRoasted();
    [[nodiscard]] bool HasOnion() const;
    // Добавляем лук
    void AddOnion();
    [[nodiscard]] bool IsPacked() const;
    void Pack();

private:
    // Убеждаемся, что котлета прожарена
    void AssureCutletRoasted() const;

    bool cutlet_roasted_ = false; // Обжарена ли котлета?
    bool has_onion_ = false;      // Есть ли лук?
    bool is_packed_ = false;      // Упакован ли гамбургер?
};

std::ostream &operator<<(std::ostream &os, const Hamburger &h);