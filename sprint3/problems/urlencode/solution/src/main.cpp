#include <iostream>

#include "urlencode.h"

int main() {
    std::string s;
    s += static_cast<unsigned char>(128);

    std::cout << UrlEncode(s) << std::endl;
}
