#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

#include "../src/collision_detector.h"


// Напишите здесь тесты для функции collision_detector::FindGatherEvents

namespace Catch {
template<>
struct StringMaker<collision_detector::GatheringEvent> {
    static std::string convert(collision_detector::GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

        return tmp.str();
    }
};

}

using namespace collision_detector;
using namespace Catch::Matchers;

class TestProvider : public ItemGathererProvider {
public:
    TestProvider(std::vector<Item> items, std::vector<Gatherer> gatherers)
        : items_(std::move(items)), gatherers_(std::move(gatherers)) {}

    size_t ItemsCount() const override { return items_.size(); }
    Item GetItem(size_t idx) const override { return items_.at(idx); }
    size_t GatherersCount() const override { return gatherers_.size(); }
    Gatherer GetGatherer(size_t idx) const override { return gatherers_.at(idx); }

private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};

const double EPS = 1e-10;

TEST_CASE("One Dog One Item One Colision", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{-5.0, 0.0}, {5.0, 0.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 1);
    CHECK(events[0].item_id == 0);
    CHECK(events[0].gatherer_id == 0);
    CHECK_THAT(events[0].sq_distance, WithinAbs(0, EPS));
    CHECK_THAT(events[0].time, WithinRel(0.5,EPS));
}

TEST_CASE("One Dog One Item No Colision", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{-5.0, 0.0}, {-1.0, 0.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 0);
}

TEST_CASE("One Dog One Item No Colision VeryClose", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{-5.0, 0.0}, {-EPS, 0.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 0);
}

TEST_CASE("One Dog One Item Colision SmallContact", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{-5.0, 0.0}, {EPS, 0.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 1);
    CHECK(events[0].item_id == 0);
    CHECK(events[0].gatherer_id == 0);
    CHECK_THAT(events[0].sq_distance, WithinAbs(0, EPS));
    CHECK_THAT(events[0].time, WithinRel(1,EPS));  //Сомнительно
}

TEST_CASE("One Dog One Item ColisionAtStart", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{0.0, 0.0}, {1.0, 0.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 1);
    CHECK(events[0].item_id == 0);
    CHECK(events[0].gatherer_id == 0);
    CHECK_THAT(events[0].sq_distance, WithinAbs(0, EPS));
    CHECK_THAT(events[0].time, WithinRel(0,EPS));  //Сомнительно
}

TEST_CASE("One Dog One Item NoColision SmallDistance", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{EPS, 0.0}, {1.0, 0.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 0);
}

TEST_CASE("One Dog Three Item Tree Colision", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    std::vector<Item> items = {
        {{-5.0, 0.0}, 1.0},
        {{0.0, 0.0}, 1.0},
        {{5.0, 0.0}, 1.0}
    };
    Gatherer gatherer{{-5.0, 0.0}, {5.0, 0.0}, 1.0};
    TestProvider provider({items}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 3);

    CHECK(events[0].item_id == 0);
    CHECK(events[0].gatherer_id == 0);
    CHECK_THAT(events[0].sq_distance, WithinAbs(0,EPS));
    CHECK_THAT(events[0].time, WithinRel(0,EPS));

    CHECK(events[1].item_id == 1);
    CHECK(events[1].gatherer_id == 0);
    CHECK_THAT(events[1].sq_distance, WithinAbs(0, EPS));
    CHECK_THAT(events[1].time, WithinRel(0.5,EPS));

    CHECK(events[2].item_id == 2);
    CHECK(events[2].gatherer_id == 0);
    CHECK_THAT(events[2].sq_distance, WithinAbs(0, EPS));
    CHECK_THAT(events[2].time, WithinRel(1,EPS));
}

TEST_CASE("One Dog OneItem NoEnoughWigth", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.7}, 0.4};
    Gatherer gatherer{{-5.0, -0.4}, {5.0, -0.4}, 0.5};  // Не касаются друг друга из за ширины
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 0);
}

TEST_CASE("One Dog OneItem SmallContact", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.7}, 0.4};
    Gatherer gatherer{{-5.0, 0}, {5.0, 0}, 0.3};  //Коснулись ту мало
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 1);
}

TEST_CASE("One Dog OneItem SmallDistance", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item{{0.0, 0.7}, 0.4};
    Gatherer gatherer{{-5.0, 0}, {5.0, 0}, 0.3 -EPS};  //Нет касания
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 0);
}

TEST_CASE("Two Dog One Item Two Colision", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item ={{0.0, 0.0}, 1.0};

    Gatherer gatherer{{-5.0, 0.0}, {5.0, 0.0}, 1.0};
    Gatherer gatherer_clone{{-1.0, 0.0}, {3.0, 0.0}, 1.0};
    std::vector<Gatherer> gatherers;
    gatherers.push_back(gatherer);
    gatherers.push_back(gatherer_clone);

    TestProvider provider({item}, {gatherers});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 2);

    CHECK(events[0].item_id == 0);
    CHECK(events[0].gatherer_id == 1);
    CHECK_THAT(events[0].sq_distance, WithinAbs(0,EPS));
    CHECK_THAT(events[0].time, WithinRel(0.25,EPS));

    CHECK(events[1].item_id == 0);
    CHECK(events[1].gatherer_id == 0);
    CHECK_THAT(events[1].sq_distance, WithinAbs(0,EPS));
    CHECK_THAT(events[1].time, WithinRel(0.5,EPS));
}

TEST_CASE("Two Dog TWO Item Two Colision", "[collision]") {
    // Предмет в центре, собиратель движется прямо через него
    Item item ={{-2.0, 0.0}, 1.0};
    Item item_clone ={{3.0, 0.0}, 1.0};

    std::vector<Item> items;
    items.push_back(item);
    items.push_back(item_clone);

    Gatherer gatherer{{-5.0, 0.0}, {1.0, 0.0}, 1.0};
    Gatherer gatherer_clone{{1.0, 0.0}, {5.0, 0.0}, 1.0};
    std::vector<Gatherer> gatherers;
    gatherers.push_back(gatherer);
    gatherers.push_back(gatherer_clone);

    TestProvider provider({items}, {gatherers});
    auto events = FindGatherEvents(provider);

    REQUIRE(events.size() == 2);

    CHECK(events[0].item_id == 0);
    CHECK(events[0].gatherer_id == 0);
    CHECK_THAT(events[0].sq_distance, WithinAbs(0,EPS));
    CHECK_THAT(events[0].time, WithinRel(0.5,EPS));

    CHECK(events[1].item_id == 1);
    CHECK(events[1].gatherer_id == 1);
    CHECK_THAT(events[1].sq_distance, WithinAbs(0,EPS));
    CHECK_THAT(events[1].time, WithinRel(0.5,EPS));
}

TEST_CASE("zero movement ignored", "[collision]") {
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{0.0, 0.0}, {0.0, 0.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);
    CHECK(events.empty());
}

TEST_CASE(" diagonal movement", "[collision]") {
    Item item{{0.0, 0.0}, 1.0};
    Gatherer gatherer{{-5.0, -5.0}, {5.0, 5.0}, 1.0};
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);
    REQUIRE(events.size() == 1);
    // Расстояние от точки до прямой (y=x) равно 0, проекция в середине => время 0.5
    CHECK_THAT(events[0].time, WithinRel(0.5, EPS));
    CHECK_THAT(events[0].sq_distance, WithinAbs(0.0, EPS));
}

TEST_CASE("different wight", "[collision]") {
    Item item{{0.0, 1.5}, 0.5};   // радиус 0.5
    Gatherer gatherer{{-5.0, 0.0}, {5.0, 0.0}, 1.0}; // радиус 1.0
    // Сумма радиусов = 1.5, расстояние до прямой = 1.5 => касание
    TestProvider provider({item}, {gatherer});
    auto events = FindGatherEvents(provider);
    REQUIRE(events.size() == 1);
    CHECK_THAT(events[0].sq_distance, WithinRel(2.25, EPS)); // (1.5)^2 = 2.25
    CHECK_THAT(events[0].time, WithinRel(0.5, EPS));
}



















