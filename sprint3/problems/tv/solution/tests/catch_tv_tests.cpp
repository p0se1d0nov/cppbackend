#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <stdexcept>

#include "../src/tv.h"

namespace Catch {

template <>
struct StringMaker<std::nullopt_t> {
    static std::string convert(std::nullopt_t) {
        using namespace std::literals;
        return "nullopt"s;
    }
};

template <typename T>
struct StringMaker<std::optional<T>> {
    static std::string convert(const std::optional<T>& opt_value) {
        if (opt_value) {
            return StringMaker<T>::convert(*opt_value);
        } else {
            return StringMaker<std::nullopt_t>::convert(std::nullopt);
        }
    }
};

}  // namespace Catch

SCENARIO("TV", "[TV]") {
    GIVEN("A TV") {
        TV tv;

        SECTION("Initially it is off and doesn't show any channel") {
            CHECK(!tv.IsTurnedOn());
            CHECK(!tv.GetChannel().has_value());
        }

        WHEN("it is turned off") {
            REQUIRE(!tv.IsTurnedOn());

            THEN("it can't select any channel") {
                CHECK_THROWS_AS(tv.SelectChannel(10), std::logic_error);
                CHECK(tv.GetChannel() == std::nullopt);
                tv.TurnOn();
                CHECK(tv.GetChannel() == 1);
            }
        }

        WHEN("it is turned on first time") {
            tv.TurnOn();

            THEN("it is turned on and shows channel #1") {
                CHECK(tv.IsTurnedOn());
                CHECK(tv.GetChannel() == 1);

                AND_WHEN("it is turned off") {
                    tv.TurnOff();
                    THEN("it is turned off and doesn't show any channel") {
                        CHECK(!tv.IsTurnedOn());
                        CHECK(tv.GetChannel() == std::nullopt);
                    }
                }
            }

            AND_THEN("it can select channel from 1 to 99") {
                tv.SelectChannel(5);
                CHECK(tv.GetChannel() == 5);
                tv.SelectChannel(99);
                CHECK(tv.GetChannel() == 99);

                tv.SelectChannel(42);
                CHECK(tv.GetChannel() == 42);
                tv.SelectChannel(42);  // повторный выбор того же канала
                CHECK(tv.GetChannel() == 42);

                CHECK_THROWS_AS(tv.SelectChannel(0), std::out_of_range);
                CHECK_THROWS_AS(tv.SelectChannel(100), std::out_of_range);
                CHECK_THROWS_AS(tv.SelectChannel(-5), std::out_of_range);
                CHECK(tv.GetChannel() == 42);
            }

            AND_THEN("it can switch to the last viewed channel") {
                tv.SelectChannel(7);
                tv.SelectChannel(9);
                tv.SelectLastViewedChannel();
                CHECK(tv.GetChannel() == 7);
                tv.SelectLastViewedChannel();
                CHECK(tv.GetChannel() == 9);

                TV tv2;
                tv2.TurnOn();
                tv2.SelectLastViewedChannel();
                CHECK(tv2.GetChannel() == 1);
                tv2.SelectLastViewedChannel();
                CHECK(tv2.GetChannel() == 1);
            }

            AND_THEN("it remembers the last channel after turning off and on") {
                tv.SelectChannel(15);
                tv.TurnOff();
                CHECK(!tv.IsTurnedOn());
                tv.TurnOn();
                CHECK(tv.IsTurnedOn());
                CHECK(tv.GetChannel() == 15);
            }
        }
    }
}
