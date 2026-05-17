#include <gtest/gtest.h>

#include "../src/urlencode.h"



using namespace std::literals;




TEST(UrlEncodeTestSuite, Empty) {
    //Пустая входная строка
    EXPECT_EQ(UrlEncode(""sv), ""s);

}

TEST(UrlEncodeTestSuite, Without) {


    // Входная строка без служебных символов
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
    EXPECT_EQ(UrlEncode("Hello"sv), "Hello"s);


}

TEST(UrlEncodeTestSuite, Space) {


    //Входная строка с пробелами
    EXPECT_EQ(UrlEncode("Hello World"sv),"Hello+World"s);
    EXPECT_EQ(UrlEncode("Hello World "sv), "Hello+World+"s);


}

TEST(UrlEncodeTestSuite, WithNorm) {
    //Входная строка со служебными символами
    EXPECT_EQ(UrlEncode("Hello!"sv), "Hello%21"s);
    EXPECT_EQ(UrlEncode("Hello*World"sv), "Hello%2AWorld"s);
    EXPECT_EQ(UrlEncode("Hello* World"sv), "Hello%2A+World"s);
    EXPECT_EQ(UrlEncode("*Hello*"sv), "%2AHello%2A"s);



}

TEST(UrlEncodeTestSuite, With31AND128) {
    std::string str80;
    std::string strD4;
    std::string str9F;
    std::string strA8;

    str80 += static_cast<unsigned char>(128);
    strD4 += static_cast<char>(212);
    str9F += static_cast<char>(159);
    strA8 += static_cast<char>(168);

    EXPECT_EQ(UrlEncode(strD4), "%D4"s);
    EXPECT_EQ(UrlEncode(str9F), "%9F"s);
    EXPECT_EQ(UrlEncode(str80), "%80"s);
    EXPECT_EQ(UrlEncode(strA8), "%A8"s);

    //Входная строка с символами с кодами меньше 31 и большими или равными 128

}

/* Напишите остальные тесты самостоятельно */
