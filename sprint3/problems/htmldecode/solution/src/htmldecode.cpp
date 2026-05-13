#include "htmldecode.h"
#include <sstream>
//#include <iostream>
/*
 * ƒекодирует основные HTML-мнемоники:
 * - &lt - <
 * - &gt - >
 * - &amp - &
 * - &pos - '
 * - &quot - "
 *
 * ћнемоника может быть записана целиком либо строчными, либо заглавными буквами:
 * - &lt и &LT декодируютс€ как <
 * - &Lt и &lT не мнемоники
 *
 * ѕосле мнемоники может сто€ть опциональный символ ;
 * - M&amp;M&APOSs декодируетс€ в M&M's
 * - &amp;lt; декодируетс€ в &lt;
 */

std::string HtmlDecode(std::string_view str) {
    // Ќапишите недостающий код самосто€тельно
    std::ostringstream result;
    for (auto i = 0; i < str.size(); ++i) {
        if (str[i] == '&') {
            std::string s2 = std::string(str.substr(i+1,2));
            std::string s3 = std::string(str.substr(i+1,3));
            std::string s4 = std::string(str.substr(i+1,4));
            if (s2 == "lt" || s2 =="gt") {
                if (s2[0] == 'l') {
                    result << '<';
                }
                else {
                   result << '>';
                }
                i=i+2;
            }
            else if (s2 == "LT" || s2 =="GT") {
                if (s2[0] == 'L') {
                    result << '<';
                }
                else {
                    result << '>';
                }
                i=i+3;
            }
            else if (s3 == "amp" || s3 == "pos") {
                if (s3[0] == 'a') {
                    result << "&";
                }
                else {
                    result << '\'';
                }
                i=i+3;
            }
            else if (s3 == "AMP" || s3== "POS") {
                if (s3[0] == 'A') {
                    result << "&";
                }
                else {
                    result << '\'';
                }
                i=i+3;
            }
            else if (s4 == "quot" || s4 == "QUOT") {
                result << '"';
                i = i + 4;
            }
            else {
                result << str[i];
            }
        }
        else {
            result << str[i];
        }
    }
    return result.str();
}
