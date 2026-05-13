#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);

    //Строка без %последовательностей
    BOOST_TEST(UrlDecode("Hello"sv) == "Hello"s);
    BOOST_TEST(UrlDecode("Hello R"sv) == "Hello R"s);

    //Строка с символом +
    BOOST_TEST(UrlDecode("Hello+World"sv) == "Hello World"s);
    BOOST_TEST(UrlDecode("Hello+World+"sv) == "Hello World "s);

    //Строка с валидными %-последовательностями, записанными в разном регистре.
    BOOST_TEST(UrlDecode("Hello%20World"sv) == "Hello World"s);
    BOOST_TEST(UrlDecode("Hello%21"sv) == "Hello!"s);
    BOOST_TEST(UrlDecode("Hello%2BWorld"sv) == "Hello+World"s);
    BOOST_TEST(UrlDecode("Hello%2bWorld"sv) == "Hello+World"s);

    BOOST_TEST(UrlDecode("Hello%2AWorld"sv) == "Hello*World"s);
    BOOST_TEST(UrlDecode("Hello%2aWorld"sv) == "Hello*World"s);

    //Строка с невалидными %-последовательностями.
    BOOST_CHECK_THROW(UrlDecode("Hello%GGWorld"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("Hello%World"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("Hello%%World"sv), std::invalid_argument);

    //Строка с неполными %-последовательностями.
    BOOST_CHECK_THROW(UrlDecode("Hello%%World"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("World%"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("World%f"sv), std::invalid_argument);


    // Напишите остальные тесты для функции UrlDecode самостоятельно
}
