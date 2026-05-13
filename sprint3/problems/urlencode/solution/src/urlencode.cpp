#include "urlencode.h"
#include <sstream>
#include <iomanip>
#include <iostream>

/*
 * URL-кодирует строку str.
 * Пробел заменяется на +,
 * Символы, отличные от букв английского алфавита, цифр и -._~ а также зарезервированные символы
 * заменяются на их %-кодированные последовательности.
 * Зарезервированные символы: !#$&'()*+,/:;=?@[]
 */

bool NeedToEncode(unsigned char c) {
    return (c <=31 || c >= 128 ||  c == '!' || c == '#' || c == '$' || c == '&' || c == '\'' ||
            c == '(' || c == ')' || c == '*' || c == '+' || c == ',' ||
            c == '/' || c == ':' || c == ';' || c == '=' || c == '?' ||
            c == '@' || c == '[' || c == ']');
}


std::string UrlEncode(std::string_view str) {
    // Напишите реализацию самостоятельно
    std::ostringstream encoded;

    for (unsigned char c: str) {
        if (c == ' ') {
            encoded << '+';
        }
        else if (NeedToEncode(c)) {
            encoded << '%' << std::uppercase << std::hex <<  std::setw(2) << std::setfill('0') << (unsigned int)(c);

        }
        else {
            encoded << static_cast<char>(c);
        }

    }

    return encoded.str();
}
