#include "collision_detector.h"
#include <cassert>

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider &provider)
{
    std::vector<GatheringEvent> result;
    if (provider.ItemsCount() == 0) {
        return result;
    }
    for (size_t gatherer_id = 0; gatherer_id < provider.GatherersCount(); gatherer_id++) {
        auto start = provider.GetGatherer(gatherer_id).start_pos;
        auto end = provider.GetGatherer(gatherer_id).end_pos;
        auto gatherer_width = provider.GetGatherer(gatherer_id).width;
        auto is_move = start <=> end;

        if (is_move == 0) {
            continue;
        }
        for (size_t item_id = 0; item_id < provider.ItemsCount(); item_id++) {
            auto item_pos = provider.GetItem(item_id).position;
            auto item_width = provider.GetItem(item_id).width;
            const double u_x = item_pos.x - start.x;
            const double u_y = item_pos.y - start.y;
            const double v_x = end.x - start.x;
            const double v_y = end.y - start.y;
            const double u_dot_v = u_x * v_x + u_y * v_y;
            const double u_len2 = u_x * u_x + u_y * u_y;
            const double v_len2 = v_x * v_x + v_y * v_y;
            const double proj_ratio = u_dot_v / v_len2;
            const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

            double collection_radius = (gatherer_width + item_width);

            if (proj_ratio >= 0.0 && proj_ratio <= 1.0 && sq_distance <= collection_radius * collection_radius){
                result.push_back({item_id,gatherer_id,sq_distance,proj_ratio});
            }
        }
    }
    std::sort(result.begin(), result.end(),
              [](const GatheringEvent& a, const GatheringEvent& b) {
                  return a.time < b.time;
              });
    return result;

}
// В задании на разработку тестов реализовывать следующую функцию не нужно -
// она будет линковаться извне.

}  // namespace collision_detector
