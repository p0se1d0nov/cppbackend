#include "urldecode.h"

#include <charconv>
#include <stdexcept>
#include <sstream>

std::string UrlDecode(std::string_view str){
    std::string result;

    for (size_t i = 0; i < str.size(); i++) {  // Проходим по req.target()
        if (str[i] == '%' ) {  // Если видим %
            if (i+2 >= str.size()) {
                throw std::invalid_argument("Invalid argument");
            }
            int val = 0;
            std::string hex{str.substr(i+1,2)};  // Берем 2 char после %
            std::istringstream iss(hex);
            iss >> std::hex >> val; // пытаемся преобразовать
            if (!iss.fail()) {
                result += static_cast<char>(val);  // преобразовываем в char Например  (%2b == +) вернется true
                i +=2;   // Увеличиваем индекс потосу что мы уже прочли их
            } else {
                throw std::invalid_argument("Invalid argument %" + hex);
                //result += '%';  // если не вышло кидаем симовл '%'
            }

        } else {
            if (str[i] == '+') {
                result += ' ';
            }
            else {
                result +=str[i];    // если не % просто добавляем тек. char
            }

        }
    }
    return result;
}
