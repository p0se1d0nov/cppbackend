#pragma once

#include "game.h"
#include "iroute.h"

namespace Routes {

class MapRoute : public IRoute {

public:
  explicit MapRoute(model::Game &game) : IRoute{game} {};

  MapRoute(const MapRoute &) = delete;
  MapRoute &operator=(const MapRoute &) = delete;

  http_handler::LogResponseData operator()(const It begin, const It end,
                                           const Method method,
                                           const Request &req) const override;

private:
  boost::json::array GetArrayJsonMapHeads() const;

  boost::json::object GetObjectJsonFromMap(const model::Map *const map) const;

  boost::json::object GreateJsonMap(const model::Map *const map) const;

  void AddRoadsInJsonObject(boost::json::object &json_map,
                            const model::Map *const map) const;

  void AddBuildingsInJsonObject(boost::json::object &json_map,
                                const model::Map *const map) const;

  void AddOfficesInJsonObject(boost::json::object &json_map,
                              const model::Map *const map) const;
};
} // namespace Routes