#pragma once

#include <boost/json/array.hpp>
#include <unordered_map>
#include "model.h"

namespace extra_data {

class ExtraDataRepository {
public:
    using MapId = model::Map::Id;

    static ExtraDataRepository& GetInstance() {
        static ExtraDataRepository instance;
        return instance;
    }

    void SetLootTypes(MapId id, boost::json::array loot_types);
    const boost::json::array* GetLootTypes(MapId id) const;

    void Clear();

private:
    ExtraDataRepository() = default;
    std::unordered_map<MapId, boost::json::array, util::TaggedHasher<MapId>> loot_types_map_;
};

} // namespace extra_data
