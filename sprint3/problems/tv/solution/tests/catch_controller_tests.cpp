#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <sstream>

#include "../src/controller.h"

SCENARIO("Controller", "[Controller]") {
    using namespace std::literals;
    GIVEN("Controller and TV") {
        TV tv;
        std::istringstream input;
        std::ostringstream output;
        Menu menu{input, output};
        Controller controller{tv, menu};

        auto run_menu_command = [&menu, &input](std::string command) {
            input.str(std::move(command));
            input.clear();
            menu.Run();
        };
        auto expect_output = [&output](std::string_view expected) {
            CHECK(output.str() == std::string{expected});
        };
        auto expect_extra_arguments_error = [&expect_output](std::string_view command) {
            expect_output("Error: the "s.append(command).append(
                " command does not require any arguments\n"sv));
        };
        auto expect_empty_output = [&expect_output] {
            expect_output({});
        };
        auto clear_output = [&output] { output.str(""); output.clear(); };

        WHEN("The TV is turned off") {
            AND_WHEN("Info command is entered without arguments") {
                run_menu_command("Info"s);
                THEN("output contains info that TV is turned off") {
                    expect_output("TV is turned off\n"s);
                }
            }

            AND_WHEN("Info command is entered with some arguments") {
                run_menu_command("Info some extra arguments");
                THEN("Error message is printed") {
                    expect_extra_arguments_error("Info"s);
                }
            }

            AND_WHEN("Info command has trailing spaces") {
                run_menu_command("Info  "s);
                THEN("output contains information that TV is turned off") {
                    expect_output("TV is turned off\n"s);
                }
            }

            AND_WHEN("TurnOn command is entered without arguments") {
                run_menu_command("TurnOn"s);
                THEN("TV is turned on") {
                    CHECK(tv.IsTurnedOn());
                    expect_empty_output();
                }
            }

            AND_WHEN("TurnOn command is entered with some arguments") {
                run_menu_command("TurnOn some args"s);
                THEN("the error message is printed and TV is not turned on") {
                    CHECK(!tv.IsTurnedOn());
                    expect_extra_arguments_error("TurnOn"s);
                }
            }

            AND_WHEN("SelectChannel command is entered") {
                AND_WHEN("channel number is valid") {
                    run_menu_command("SelectChannel 5"s);
                    THEN("TV stays off and error message about being off is printed") {
                        CHECK(!tv.IsTurnedOn());
                        expect_output("TV is turned off\n"s);
                    }
                }
                AND_WHEN("channel number is not an integer") {
                    run_menu_command("SelectChannel abc"s);
                    THEN("error 'Invalid channel' is printed") {
                        CHECK(!tv.IsTurnedOn());
                        expect_output("Invalid channel\n"s);
                    }
                }
            }

            AND_WHEN("SelectPreviousChannel command is entered") {
                run_menu_command("SelectPreviousChannel"s);
                THEN("error 'TV is turned off' is printed") {
                    expect_output("TV is turned off\n"s);
                }

                // AND_WHEN("with extra arguments") {
                //     run_menu_command("SelectPreviousChannel arg"s);
                //     THEN("error about extra arguments is printed") {
                //         expect_extra_arguments_error("SelectPreviousChannel"s);
                //     }
                // }
            }
        }

        WHEN("The TV is turned on") {
            tv.TurnOn();
            AND_WHEN("TurnOff command is entered without arguments") {
                run_menu_command("TurnOff"s);
                THEN("TV is turned off") {
                    CHECK(!tv.IsTurnedOn());
                    expect_empty_output();
                }
            }

            AND_WHEN("TurnOff command is entered with some arguments") {
                run_menu_command("TurnOff some args");
                THEN("the error message is printed and TV is not turned off") {
                    CHECK(tv.IsTurnedOn());
                    expect_extra_arguments_error("TurnOff"s);
                }
            }

            AND_WHEN("Info command is entered without arguments") {
                tv.SelectChannel(12);
                run_menu_command("Info"s);
                THEN("current channel is printed") {
                    expect_output("TV is turned on\nChannel number is 12\n"s);
                }
            }

            AND_WHEN("Info command is entered with extra arguments") {
                run_menu_command("Info extra"s);
                THEN("error about extra arguments is printed") {
                    expect_extra_arguments_error("Info"s);
                }
            }

            AND_WHEN("SelectChannel command is entered") {
                AND_WHEN("channel number is valid") {
                    run_menu_command("SelectChannel 42"s);
                    THEN("channel is changed and no output") {
                        CHECK(tv.GetChannel() == 42);
                        expect_empty_output();
                    }
                }

                // Исправленная секция: разбиваем на два независимых SECTION
                AND_WHEN("channel number is out of range") {
                    SECTION("channel 0") {
                        run_menu_command("SelectChannel 0"s);
                        THEN("error 'Channel is out of range' is printed") {
                            expect_output("Channel is out of range\n"s);
                            CHECK(tv.GetChannel() == 1);
                        }
                    }
                    SECTION("channel 100") {
                        run_menu_command("SelectChannel 100"s);
                        THEN("error 'Channel is out of range' is printed") {
                            expect_output("Channel is out of range\n"s);
                            CHECK(tv.GetChannel() == 1);
                        }
                    }
                }

                AND_WHEN("channel number is not an integer") {
                    run_menu_command("SelectChannel abc"s);
                    THEN("error 'Invalid channel' is printed") {
                        expect_output("Invalid channel\n"s);
                        CHECK(tv.GetChannel() == 1);
                    }
                }

                AND_WHEN("channel number is missing") {
                    run_menu_command("SelectChannel"s);
                    THEN("error 'Invalid channel' is printed") {
                        expect_output("Invalid channel\n"s);
                    }
                }
            }

            AND_WHEN("SelectPreviousChannel command is entered") {
                // Подготовка: два выбранных канала
                tv.SelectChannel(7);
                tv.SelectChannel(9);  // текущий 9, предыдущий 7

                SECTION("switches to previous channel with no output") {
                    run_menu_command("SelectPreviousChannel"s);
                    CHECK(tv.GetChannel() == 7);
                    expect_empty_output();
                }

                SECTION("when called again, toggles back") {
                    run_menu_command("SelectPreviousChannel"s);
                    CHECK(tv.GetChannel() == 7);
                    run_menu_command("SelectPreviousChannel"s);
                    CHECK(tv.GetChannel() == 9);
                    expect_empty_output();
                }
            }

            // AND_WHEN("SelectPreviousChannel command is entered with extra arguments") {
            //     run_menu_command("SelectPreviousChannel arg"s);
            //     THEN("error about extra arguments is printed") {
            //         expect_extra_arguments_error("SelectPreviousChannel"s);
            //     }
            // }
        }
    }
}
