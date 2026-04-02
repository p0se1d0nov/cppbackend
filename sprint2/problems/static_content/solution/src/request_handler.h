#pragma once
#include "http_server.h"
#include "model.h"

#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <boost/json.hpp>

namespace http_handler
{
    namespace json = boost::json;
    namespace beast = boost::beast;
    namespace http = beast::http;
    using namespace std::literals;

    // Вспомогательные функции для создания JSON ответов
    namespace
    {

        json::object RoadToJson(const model::Road &road)
        {
            json::object obj;
            obj["x0"] = road.GetStart().x;
            obj["y0"] = road.GetStart().y;

            if (road.IsHorizontal())
            {
                obj["x1"] = road.GetEnd().x;
            }
            else
            {
                obj["y1"] = road.GetEnd().y;
            }

            return obj;
        }

        json::object BuildingToJson(const model::Building &building)
        {
            const auto &bounds = building.GetBounds();
            json::object obj;
            obj["x"] = bounds.position.x;
            obj["y"] = bounds.position.y;
            obj["w"] = bounds.size.width;
            obj["h"] = bounds.size.height;
            return obj;
        }

        json::object OfficeToJson(const model::Office &office)
        {
            json::object obj;
            obj["id"] = *office.GetId();
            obj["x"] = office.GetPosition().x;
            obj["y"] = office.GetPosition().y;
            obj["offsetX"] = office.GetOffset().dx;
            obj["offsetY"] = office.GetOffset().dy;
            return obj;
        }

        json::object MapToJson(const model::Map &map)
        {
            json::object obj;
            obj["id"] = *map.GetId();
            obj["name"] = map.GetName();

            // Добавляем дороги
            json::array roads_json;
            for (const auto &road : map.GetRoads())
            {
                roads_json.push_back(RoadToJson(road));
            }
            obj["roads"] = roads_json;

            // Добавляем здания
            json::array buildings_json;
            for (const auto &building : map.GetBuildings())
            {
                buildings_json.push_back(BuildingToJson(building));
            }
            obj["buildings"] = buildings_json;

            // Добавляем офисы
            json::array offices_json;
            for (const auto &office : map.GetOffices())
            {
                offices_json.push_back(OfficeToJson(office));
            }
            obj["offices"] = offices_json;

            return obj;
        }

        // Парсит путь из запроса
        // Возвращает пустую строку, если путь не содержит параметр
        std::string ExtractMapIdFromPath(const std::string &path)
        {
            // Ожидаем формат: /api/v1/maps/{map_id}
            const std::string prefix = "/api/v1/maps/";
            if (path.size() > prefix.size() && path.substr(0, prefix.size()) == prefix)
            {
                return path.substr(prefix.size());
            }
            return "";
        }

        // Создаёт JSON ответ об ошибке
        json::object ErrorResponse(const std::string &code, const std::string &message)
        {
            json::object obj;
            obj["code"] = code;
            obj["message"] = message;
            return obj;
        }

        // URL-decoding path. Returns empty string in case of failure.
        std::string UrlDecode(const std::string &value, bool &ok)
        {
            std::string result;
            result.reserve(value.size());
            for (size_t i = 0; i < value.size(); ++i)
            {
                char c = value[i];
                if (c == '%')
                {
                    if (i + 2 >= value.size())
                    {
                        ok = false;
                        return "";
                    }
                    auto hex_digit = [&](char ch) -> int {
                        if (ch >= '0' && ch <= '9')
                            return ch - '0';
                        if (ch >= 'a' && ch <= 'f')
                            return ch - 'a' + 10;
                        if (ch >= 'A' && ch <= 'F')
                            return ch - 'A' + 10;
                        return -1;
                    };
                    int h = hex_digit(value[i + 1]);
                    int l = hex_digit(value[i + 2]);
                    if (h < 0 || l < 0)
                    {
                        ok = false;
                        return "";
                    }
                    result.push_back(static_cast<char>((h << 4) | l));
                    i += 2;
                }
                else if (c == '+')
                {
                    result.push_back(' ');
                }
                else
                {
                    result.push_back(c);
                }
            }
            ok = true;
            return result;
        }

        std::string MimeType(std::string extension)
        {
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
            if (!extension.empty() && extension.front() == '.')
            {
                extension.erase(extension.begin());
            }

            if (extension == "htm" || extension == "html")
                return "text/html";
            if (extension == "css")
                return "text/css";
            if (extension == "txt")
                return "text/plain";
            if (extension == "js")
                return "text/javascript";
            if (extension == "json")
                return "application/json";
            if (extension == "xml")
                return "application/xml";
            if (extension == "png")
                return "image/png";
            if (extension == "jpg" || extension == "jpe" || extension == "jpeg")
                return "image/jpeg";
            if (extension == "gif")
                return "image/gif";
            if (extension == "bmp")
                return "image/bmp";
            if (extension == "ico")
                return "image/vnd.microsoft.icon";
            if (extension == "tiff" || extension == "tif")
                return "image/tiff";
            if (extension == "svg" || extension == "svgz")
                return "image/svg+xml";
            if (extension == "mp3")
                return "audio/mpeg";

            return "application/octet-stream";
        }

        bool IsPathUnderRoot(const std::filesystem::path &path, const std::filesystem::path &root)
        {
            auto normalized_path = path.lexically_normal();
            auto normalized_root = root.lexically_normal();

            auto p_it = normalized_path.begin();
            auto r_it = normalized_root.begin();
            for (; r_it != normalized_root.end() && p_it != normalized_path.end(); ++r_it, ++p_it)
            {
                if (*r_it != *p_it)
                {
                    return false;
                }
            }
            return r_it == normalized_root.end();
        }

    } // anonymous namespace

    class RequestHandler
    {
    public:
        RequestHandler(model::Game &game, const std::string &static_root)
            : game_{game}, static_root_{std::filesystem::canonical(static_root)}
        {
        }

        RequestHandler(const RequestHandler &) = delete;
        RequestHandler &operator=(const RequestHandler &) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send)
        {
            auto make_json_response = [&req](http::status status, std::string body)
            {
                http::response<http::string_body> res{status, req.version()};
                res.set(http::field::content_type, "application/json");
                res.body() = std::move(body);
                res.prepare_payload();
                return res;
            };

            auto make_plain_response = [&req](http::status status, std::string body)
            {
                http::response<http::string_body> res{status, req.version()};
                res.set(http::field::content_type, "text/plain");
                res.body() = std::move(body);
                res.prepare_payload();
                return res;
            };

            auto make_bad_request = [&](std::string_view message = "Bad request")
            {
                return make_json_response(http::status::bad_request, json::serialize(ErrorResponse("badRequest", std::string(message))));
            };

            const auto &target = req.target();
            std::string path_str(target.begin(), target.end());

            if (path_str.rfind("/api/", 0) == 0)
            {
                // API handling
                if (path_str == "/api/v1/maps" && req.method() == http::verb::get)
                {
                    json::array maps_json;
                    for (const auto &map : game_.GetMaps())
                    {
                        json::object map_obj;
                        map_obj["id"] = *map.GetId();
                        map_obj["name"] = map.GetName();
                        maps_json.push_back(map_obj);
                    }

                    auto res = make_json_response(http::status::ok, json::serialize(maps_json));
                    send(std::move(res));
                    return;
                }

                if (path_str.rfind("/api/v1/maps/", 0) == 0 && req.method() == http::verb::get && path_str.size() > 13)
                {
                    std::string map_id = ExtractMapIdFromPath(path_str);

                    if (const model::Map *map = game_.FindMap(model::Map::Id{map_id}))
                    {
                        auto map_json = MapToJson(*map);
                        auto res = make_json_response(http::status::ok, json::serialize(map_json));
                        send(std::move(res));
                        return;
                    }
                    else
                    {
                        auto error_json = ErrorResponse("mapNotFound", "Map not found");
                        auto res = make_json_response(http::status::not_found, json::serialize(error_json));
                        send(std::move(res));
                        return;
                    }
                }

                auto res = make_bad_request();
                send(std::move(res));
                return;
            }

            // Static files for non-API paths
            if (req.method() != http::verb::get && req.method() != http::verb::head)
            {
                send(make_bad_request());
                return;
            }

            // Parse and decode URI path
            auto path_no_query = path_str;
            auto query_pos = path_no_query.find('?');
            if (query_pos != std::string::npos)
            {
                path_no_query.erase(query_pos);
            }
            auto fragment_pos = path_no_query.find('#');
            if (fragment_pos != std::string::npos)
            {
                path_no_query.erase(fragment_pos);
            }

            bool decode_ok = false;
            auto decoded_path = UrlDecode(path_no_query, decode_ok);
            if (!decode_ok || decoded_path.empty() || decoded_path[0] != '/')
            {
                send(make_plain_response(http::status::bad_request, "Invalid URL-encoded path"));
                return;
            }

            std::filesystem::path file_path = static_root_;
            std::string rel = decoded_path.substr(1);
            if (!rel.empty())
            {
                file_path /= rel;
            }

            // If request is directory (or root), serve index.html
            std::error_code ec;
            if ((decoded_path.back() == '/') || (std::filesystem::exists(file_path, ec) && std::filesystem::is_directory(file_path, ec)))
            {
                file_path /= "index.html";
            }

            auto normalized_file = std::filesystem::weakly_canonical(file_path, ec);
            if (ec || !IsPathUnderRoot(normalized_file, static_root_))
            {
                send(make_plain_response(http::status::bad_request, "Bad static file path"));
                return;
            }

            if (ec || !std::filesystem::exists(normalized_file, ec) || std::filesystem::is_directory(normalized_file, ec))
            {
                send(make_plain_response(http::status::not_found, "Not found"));
                return;
            }

            std::ifstream input(normalized_file, std::ios::binary);
            if (!input)
            {
                send(make_plain_response(http::status::not_found, "Not found"));
                return;
            }

            std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            const auto mime = MimeType(normalized_file.extension().string());
            const auto file_size = content.size();

            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, mime);
            res.set(http::field::content_length, std::to_string(file_size));
            if (req.method() == http::verb::get)
            {
                res.body() = std::move(content);
            }
            else
            {
                // HEAD: empty body
                res.body() = "";
            }

            send(std::move(res));
        }

    private:
        model::Game &game_;
        std::filesystem::path static_root_;
    };


} // namespace http_handler
