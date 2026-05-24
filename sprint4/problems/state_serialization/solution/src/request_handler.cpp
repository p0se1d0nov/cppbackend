#include "request_handler.h"
//#include <boost/json/src.hpp>
namespace http_handler {


const std::unordered_map<std::string, std::string> mime_types = {
    {".htm", "text/html"}, {".html", "text/html"},
    {".css", "text/css"}, {".txt", "text/plain"},
    {".js", "text/javascript"}, {".json", "application/json"},
    {".xml", "application/xml"}, {".png", "image/png"},
    {".jpg", "image/jpeg"}, {".jpe", "image/jpeg"}, {".jpeg", "image/jpeg"},
    {".gif", "image/gif"}, {".bmp", "image/bmp"},
    {".ico", "image/vnd.microsoft.icon"}, {".tiff", "image/tiff"},
    {".tif", "image/tiff"}, {".svg", "image/svg+xml"},
    {".svgz", "image/svg+xml"}, {".mp3", "audio/mpeg"}
};

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string UrlDecode(std::string_view target){
    std::string result;

    for (size_t i = 0; i < target.size(); i++) {  // Проходим по req.target()
        if (target[i] == '%' && i+2 < target.size()) {  // Если видим %
            int val = 0;
            std::string hex{target.substr(i+1,2)};  // Берем 2 char после %
            std::istringstream iss(hex);
            iss >> std::hex >> val; // пытаемся преобразовать
            if (!iss.fail()) {
                result += static_cast<char>(val);  // преобразовываем в char Например  (%2b == +) вернется true
                i +=2;   // Увеличиваем индекс потосу что мы уже прочли их
            } else {
                result += '%';  // если не вышло кидаем симовл '%'
            }

        } else {
            result +=target[i];    // если не % просто добавляем тек. char
        }
    }
    return result;
}

std::string GetMimeType(const std::filesystem::path path){
    std::string ext = ToLower(path.extension().string());
    auto it = mime_types.find(ext);
    return (it != mime_types.end()) ? it->second : "application/octet-stream";
}

bool IsSubPath(fs::path path, fs::path base) {
    // Приводим оба пути к каноничному виду (без . и ..)
    path = fs::weakly_canonical(path);
    base = fs::weakly_canonical(base);

    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            return false;
        }
    }
    return true;
}


boost::json::value SerializeMapShort(const model::Map& map) {
    return boost::json::value{
        {"id", *map.GetId()},
        {"name",map.GetName()}
    };
}

void SerializeRoads(const model::Map& map,boost::json::object& obj) {
    boost::json::array roads;
    for (const auto& road : map.GetRoads()) {
        boost::json::object road_obj;
        road_obj["x0"] = road.GetStart().x;
        road_obj["y0"] = road.GetStart().y;
        if (road.IsHorizontal()) {
            road_obj["x1"] = road.GetEnd().x;
        } else {
            road_obj["y1"] = road.GetEnd().y;
        }
        roads.push_back(std::move(road_obj));
    }
    obj["roads"] = std::move(roads);
}

void SerializeBuildings(const model::Map& map,boost::json::object& obj) {
    boost::json::array buildings;
    for (const auto& building : map.GetBuildings()) {
        auto bounds = building.GetBounds();
        buildings.push_back({
            {"x", bounds.position.x},
            {"y", bounds.position.y},
            {"w", bounds.size.width},
            {"h", bounds.size.height}
        });
    }
    obj["buildings"] = std::move(buildings);
}

void SerializeOffices(const model::Map& map,boost::json::object& obj) {
    boost::json::array offices;
    for (const auto& office : map.GetOffices()) {
        offices.push_back({
            {"id", *office.GetId()},
            {"x", office.GetPosition().x},
            {"y", office.GetPosition().y},
            {"offsetX", office.GetOffset().dx},
            {"offsetY", office.GetOffset().dy}
        });
    }
    obj["offices"] = std::move(offices);
}

boost::json::object SerializeMapFull(const model::Map& map) {
    boost::json::object obj;
    obj["id"] = *map.GetId();
    obj["name"] = map.GetName();

    // Дороги
    SerializeRoads(map,obj);

    // Здания
    SerializeBuildings(map,obj);

    // Офисы
    SerializeOffices(map,obj);

    return obj;
}





}  // namespace http_handler
