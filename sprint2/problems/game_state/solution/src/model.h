#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "tagged.h"

namespace model
{

    using Dimension = int;
    using Coord = Dimension;
    using Real = double;

    struct Point
    {
        Coord x, y;
    };

    struct PointF
    {
        Real x, y;
    };

    struct Size
    {
        Dimension width, height;
    };

    struct Rectangle
    {
        Point position;
        Size size;
    };

    struct Offset
    {
        Dimension dx, dy;
    };

    struct Velocity
    {
        Real dx, dy;
    };

    enum class Direction
    {
        NORTH,
        SOUTH,
        WEST,
        EAST,
    };

    class Road
    {
        struct HorizontalTag
        {
            explicit HorizontalTag() = default;
        };

        struct VerticalTag
        {
            explicit VerticalTag() = default;
        };

    public:
        constexpr static HorizontalTag HORIZONTAL{};
        constexpr static VerticalTag VERTICAL{};

        Road(HorizontalTag, Point start, Coord end_x) noexcept
            : start_{start}, end_{end_x, start.y}
        {
        }

        Road(VerticalTag, Point start, Coord end_y) noexcept
            : start_{start}, end_{start.x, end_y}
        {
        }

        bool IsHorizontal() const noexcept
        {
            return start_.y == end_.y;
        }

        bool IsVertical() const noexcept
        {
            return start_.x == end_.x;
        }

        Point GetStart() const noexcept
        {
            return start_;
        }

        Point GetEnd() const noexcept
        {
            return end_;
        }

    private:
        Point start_;
        Point end_;
    };

    class Building
    {
    public:
        explicit Building(Rectangle bounds) noexcept
            : bounds_{bounds}
        {
        }

        const Rectangle &GetBounds() const noexcept
        {
            return bounds_;
        }

    private:
        Rectangle bounds_;
    };

    class Office
    {
    public:
        using Id = util::Tagged<std::string, Office>;

        Office(Id id, Point position, Offset offset) noexcept
            : id_{std::move(id)}, position_{position}, offset_{offset}
        {
        }

        const Id &GetId() const noexcept
        {
            return id_;
        }

        Point GetPosition() const noexcept
        {
            return position_;
        }

        Offset GetOffset() const noexcept
        {
            return offset_;
        }

    private:
        Id id_;
        Point position_;
        Offset offset_;
    };

    class Player
    {
    public:
        using Id = util::Tagged<std::string, Player>;

        Player(Id id, std::string token, PointF position, Velocity speed = {0.0, 0.0}, Direction direction = Direction::NORTH) noexcept
            : id_{std::move(id)}, token_{std::move(token)}, position_{position}, speed_{speed}, direction_{direction}
        {
        }

        const Id &GetId() const noexcept
        {
            return id_;
        }

        const std::string &GetToken() const noexcept
        {
            return token_;
        }

        PointF GetPosition() const noexcept
        {
            return position_;
        }

        Velocity GetSpeed() const noexcept
        {
            return speed_;
        }

        Direction GetDirection() const noexcept
        {
            return direction_;
        }

        void SetPosition(PointF position) noexcept
        {
            position_ = position;
        }

        void SetSpeed(Velocity speed) noexcept
        {
            speed_ = speed;
        }

        void SetDirection(Direction direction) noexcept
        {
            direction_ = direction;
        }

    private:
        Id id_;
        std::string token_;
        PointF position_;
        Velocity speed_;
        Direction direction_;
    };

    class Map
    {
    public:
        using Id = util::Tagged<std::string, Map>;
        using Roads = std::vector<Road>;
        using Buildings = std::vector<Building>;
        using Offices = std::vector<Office>;

        Map(Id id, std::string name) noexcept
            : id_(std::move(id)), name_(std::move(name))
        {
        }

        const Id &GetId() const noexcept
        {
            return id_;
        }

        const std::string &GetName() const noexcept
        {
            return name_;
        }

        const Buildings &GetBuildings() const noexcept
        {
            return buildings_;
        }

        const Roads &GetRoads() const noexcept
        {
            return roads_;
        }

        const Offices &GetOffices() const noexcept
        {
            return offices_;
        }

        void AddRoad(const Road &road)
        {
            roads_.emplace_back(road);
        }

        void AddBuilding(const Building &building)
        {
            buildings_.emplace_back(building);
        }

        void AddOffice(Office office);

    private:
        using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

        Id id_;
        std::string name_;
        Roads roads_;
        Buildings buildings_;

        OfficeIdToIndex warehouse_id_to_index_;
        Offices offices_;
    };

    class Game
    {
    public:
        using Maps = std::vector<Map>;
        using Players = std::vector<Player>;

        void AddMap(Map map);
        void AddPlayer(Player player);

        const Maps &GetMaps() const noexcept
        {
            return maps_;
        }

        const Players &GetPlayers() const noexcept
        {
            return players_;
        }

        const Map *FindMap(const Map::Id &id) const noexcept
        {
            if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end())
            {
                return &maps_.at(it->second);
            }
            return nullptr;
        }

        const Player *FindPlayerByToken(const std::string &token) const noexcept
        {
            if (auto it = player_token_to_index_.find(token); it != player_token_to_index_.end())
            {
                return &players_.at(it->second);
            }
            return nullptr;
        }

    private:
        using MapIdHasher = util::TaggedHasher<Map::Id>;
        using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
        using PlayerTokenToIndex = std::unordered_map<std::string, size_t>;

        std::vector<Map> maps_;
        MapIdToIndex map_id_to_index_;
        Players players_;
        PlayerTokenToIndex player_token_to_index_;
    };

} // namespace model
