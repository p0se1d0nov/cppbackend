#include "request_handler.h"

namespace http_handler {

const std::vector<std::string_view> GetSplitUrlMembers(std::string_view view) {
  std::vector<std::string_view> url_members;

  boost::split(url_members, view, boost::is_any_of("/"));
  return url_members;
}

RequestHandler::StringResponse RequestHandler::MakeStringResponse(
    const http::status status, std::string_view body, const StringRequest &req,
    std::string_view content_type, bool is_set_body) const {
  StringResponse response(status, req.version());
  response.set(http::field::content_type, content_type);
  if (is_set_body) {
    response.body() = body;
  }
  response.content_length(body.size());
  response.keep_alive(req.keep_alive());

  return response;
}

RequestHandler::FileResponse RequestHandler::MakeFileResponse(
    const http::status status, file_body::value_type &&file,
    std::string_view content_type, const StringRequest &req) const {
  response<file_body> res;
  res.version(11); // HTTP/1.1
  res.result(status::ok);
  res.insert(field::content_type, content_type);

  res.body() = std::move(file);
  // Метод prepare_payload заполняет заголовки Content-Length и
  // Transfer-Encoding в зависимости от свойств тела сообщения
  res.prepare_payload();
  return res;
};

json::array RequestHandler::GetArrayJsonMapHeads() const {
  const auto maps = game_.GetMaps();
  json::array result_maps;
  for (const auto &map : maps) {
    result_maps.push_back({{"id"s, *map.GetId()}, {"name"s, map.GetName()}});
  }
  return result_maps;
}

json::object RequestHandler::GetObjectJsonFromMap(const Map *const map) const {
  json::object json_map = GreateJsonMap(map);

  AddRoadsInJsonObject(json_map, map);
  AddBuildingsInJsonObject(json_map, map);
  AddOfficesInJsonObject(json_map, map);

  return json_map;
}

json::object RequestHandler::GreateJsonMap(const Map *const map) const {
  const auto map_id = map->GetId();
  json::object json_map;
  json_map["id"s] = *map_id;
  json_map["name"s] = map->GetName();
  return json_map;
}

void RequestHandler::AddRoadsInJsonObject(json::object &json_map,
                                          const Map *const map) const {
  json_map["roads"] = json::array{};
  for (const auto &road : map->GetRoads()) {
    json::object road_object{};

    const auto &start = road.GetStart();
    road_object["x0"] = start.x;
    road_object["y0"] = start.y;

    const auto &end = road.GetEnd();
    if (road.IsHorizontal()) {
      road_object["x1"] = end.x;
    } else {
      road_object["y1"] = end.y;
    }

    json_map["roads"].as_array().emplace_back(std::move(road_object));
  }
}

void RequestHandler::AddBuildingsInJsonObject(json::object &json_map,
                                              const Map *const map) const {
  json_map["buildings"] = json::array{};

  for (const auto &building : map->GetBuildings()) {
    json::object building_object{};

    const auto &[position, size] = building.GetBounds();
    const auto [x, y] = position;
    const auto [w, h] = size;
    building_object["x"] = x;
    building_object["y"] = y;
    building_object["w"] = w;
    building_object["h"] = h;

    json_map["buildings"].as_array().emplace_back(std::move(building_object));
  }
}

void RequestHandler::AddOfficesInJsonObject(json::object &json_map,
                                            const Map *const map) const {
  json_map["offices"] = json::array{};
  for (const auto &office : map->GetOffices()) {
    json::object office_object{};

    office_object["id"] = *office.GetId();

    const auto [x, y] = office.GetPosition();
    office_object["x"] = x;
    office_object["y"] = y;

    const auto &[dx, dy] = office.GetOffset();
    office_object["offsetX"] = dx;
    office_object["offsetY"] = dy;

    json_map["offices"].as_array().emplace_back(std::move(office_object));
  }
}

} // namespace http_handler
