#pragma once

#include <random>
#include <vector>
#include <functional>

#include "tilemap.hpp"

struct Edge
{
    sf::Vector2i p1;
    sf::Vector2i p2;
};

struct Triangle
{
    sf::Vector2i p1;
    sf::Vector2i p2;
    sf::Vector2i p3;

    bool circumCircleContains(const sf::Vector2i &v) const
    {
        float ab = (p1.x * p1.x) + (p1.y * p1.y);
        float cd = (p2.x * p2.x) + (p2.y * p2.y);
        float ef = (p3.x * p3.x) + (p3.y * p3.y);

        float circum_x = (ab * (p3.y - p2.y) + cd * (p1.y - p3.y) + ef * (p2.y - p1.y)) / (p1.x * (p3.y - p2.y) + p2.x * (p1.y - p3.y) + p3.x * (p2.y - p1.y)) / 2.f;
        float circum_y = (ab * (p3.x - p2.x) + cd * (p1.x - p3.x) + ef * (p2.x - p1.x)) / (p1.y * (p3.x - p2.x) + p2.y * (p1.x - p3.x) + p3.y * (p2.x - p1.x)) / 2.f;
        float circum_radius = sqrtf(((p1.x - circum_x) * (p1.x - circum_x)) + ((p1.y - circum_y) * (p1.y - circum_y)));

        float dist = std::sqrt(((v.x - circum_x) * (v.x - circum_x)) + ((v.y - circum_y) * (v.y - circum_y)));
        return dist <= circum_radius;
    }

    bool containsVertex(const sf::Vector2i &v) const
    {
        return p1 == v || p2 == v || p3 == v;
    }
};

inline bool operator == (const Triangle &t1, const Triangle &t2)
{
    return	(t1.p1 == t2.p1 || t1.p1 == t2.p2 || t1.p1 == t2.p3) &&
            (t1.p2 == t2.p1 || t1.p2 == t2.p2 || t1.p2 == t2.p3) &&
            (t1.p3 == t2.p1 || t1.p3 == t2.p2 || t1.p3 == t2.p3);
}

inline bool operator == (const Edge &e1, const Edge &e2)
{
    return	(e1.p1 == e2.p1 && e1.p2 == e2.p2) ||
            (e1.p1 == e2.p2 && e1.p2 == e2.p1);
}

inline float dist(const sf::Vector2i& v1, const sf::Vector2i& v2)
{
    auto delta = v1 - v2;
	return std::sqrt(delta.x*delta.x + delta.y*delta.y);
}

std::vector<Edge> triangulate(std::vector<sf::Vector2i> points)
{
    auto min = points.front();
    auto max = min;

    for(auto& point : points)
    {
        min.x = std::min(point.x, min.x);
        min.y = std::min(point.y, min.y);
        max.x = std::max(point.x, max.x);
        max.y = std::max(point.y, max.y);
    }

    auto delta = max - min;
    int deltaMax = std::max(delta.x, delta.y);
    auto mid = (min + max) / 2;

    sf::Vector2i p1(mid.x - 20 * deltaMax, mid.y - deltaMax);
    sf::Vector2i p2(mid.x, mid.y + 20 * deltaMax);
    sf::Vector2i p3(mid.x + 20 * deltaMax, mid.y - deltaMax);

    std::vector<Triangle> triangles;
    triangles.push_back({p1, p2, p3});

    for(const auto& p : points)
    {
        std::vector<Triangle> badTriangles;
        std::vector<Edge> polygon;

        for(const auto& t : triangles)
        {
            if(t.circumCircleContains(p))
            {
                badTriangles.push_back(t);
                polygon.push_back({t.p1, t.p2});
                polygon.push_back({t.p2, t.p3});
                polygon.push_back({t.p3, t.p1});
            }
        }

        triangles.erase(std::remove_if(triangles.begin(),triangles.end(),
                                       [&badTriangles](const Triangle &t)
        {
            return std::find(badTriangles.begin(), badTriangles.end(), t) != badTriangles.end();

        }), triangles.end());

        std::vector<Edge> badEdges;
        for(auto e1 = polygon.begin(); e1 != polygon.end(); e1++)
        {
            for(auto e2 = e1 + 1; e2 != polygon.end(); e2++)
            {
                if(*e1 == *e2)
                {
                    badEdges.push_back(*e1);
                    badEdges.push_back(*e2);
                }
            }
        }

        polygon.erase(std::remove_if(begin(polygon), end(polygon),
                                     [&badEdges](const Edge &e)
        {
            return std::find(badEdges.begin(), badEdges.end(), e) != badEdges.end();
        }), polygon.end());

        for(const auto& e : polygon)
            triangles.push_back({e.p1, e.p2, p});
    }

    triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
                                   [&](const Triangle &t)
    {
        return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
    }), triangles.end());


    std::vector<Edge> edges;
    for(const auto& t : triangles)
    {
        edges.push_back({t.p1, t.p2});
        edges.push_back({t.p2, t.p3});
        edges.push_back({t.p3, t.p1});
    }

    return edges;
}

std::vector<Edge> minimumSpanningTree(std::vector<Edge> edges)
{
    std::vector<sf::Vector2i> nodes;
    for(const auto& edge : edges)
    {
        if(std::find(nodes.begin(), nodes.end(), edge.p1) == nodes.end())
            nodes.push_back(edge.p1);

        if(std::find(nodes.begin(), nodes.end(), edge.p2) == nodes.end())
            nodes.push_back(edge.p2);
    }
    int uniqueCount = nodes.size();

    std::vector<std::pair<int, int>> nodesEdges;
    std::vector<std::pair<int, int>> nodesEdgesFinal;
    for(const auto& edge : edges)
    {
        nodesEdges.emplace_back(
            std::find(nodes.begin(), nodes.end(), edge.p1) - nodes.begin(),
            std::find(nodes.begin(), nodes.end(), edge.p2) - nodes.begin()
        );
    }

    std::sort(nodesEdges.begin(), nodesEdges.end(),
              [&](const auto& n1, const auto& n2)
    {
        return dist(nodes[n1.first], nodes[n1.second]) < dist(nodes[n2.first], nodes[n2.second]);
    });

    std::vector<int> fathers;
    for(int x = 0; x < uniqueCount; x++)
        fathers.push_back(x);

    std::function<int(int)> find = [&](int x)
    {
        if(fathers[x] != x)
            fathers[x] = find(fathers[x]);

        return fathers[x];
    };

    for(const auto& edge : nodesEdges)
    {
        if(find(edge.first) == find(edge.second))
            continue;

        nodesEdgesFinal.emplace_back(edge.first, edge.second);

        fathers[find(edge.first)] = find(edge.second);
    }

    std::vector<Edge> edgesFinal;
    for(const auto& edge : nodesEdgesFinal)
        edgesFinal.push_back({nodes[edge.first], nodes[edge.second]});

    return edgesFinal;
}

struct Dungeon
{
    int seed = 39129; // -1 for random;

    int roomSizeMin = 3;
    int roomSizeMax = 6;

    sf::Vector2i size = {50, 50};

    int roomPoolSize = 50;

    int minimalDirectionalRoomDistance = 7;
    int minimalRoomDistance = 3;

    int additionalEdge = 3;

    int minDoorDistToCorner = 1; //minimal distance betwindoweem corner and door, used so door don't spawindown on corner

    struct Room
    {
        sf::Vector2i pos;
        sf::Vector2i size;

        enum Side {UP, DOWN, LEFT, RIGHT};

        std::array<sf::Vector2i, 4> doors = {};
    };

    struct Corridor
    {
        sf::Vector2i start;
        sf::Vector2i end;
    };

    std::vector<Room> rooms;
    std::vector<Corridor> corridors;

    std::vector<Edge> edges;
};

inline void generateDungeon(Dungeon& dungeon)
{
    using RndEngine = std::mt19937;
    using RndDist = std::uniform_int_distribution<RndEngine::result_type>;

    RndEngine rng;
    rng.seed(dungeon.seed == -1 ? std::random_device()() : dungeon.seed);

    RndDist roomSizeDist(dungeon.roomSizeMin, dungeon.roomSizeMax);

    TileMap<bool> map;
    map.setSize({dungeon.size.x, dungeon.size.y});

    RndDist roomPosDist(0, map.getSize().x);

    auto rnd =
        [&](int min, int max)
    {
        return RndDist(min, max)(rng);
    };

    auto placeRoom =
        [&](auto pos, auto size)
    {
        if(size.x < 0)
        {
            size.x = std::abs(size.x);
            pos.x -= size.x;
        }

        if(size.y < 0)
        {
            size.y = std::abs(size.y);
            pos.y -= size.y;
        }

        for(int x = 0; x < size.x; x++)
        {
            for(int y = 0; y < size.y; y++)
            {
                map.setTile({x + pos.x, y + pos.y}, true);
            }
        }
    };

    auto canPlaceRoom =
        [&](auto pos, auto size)
    {
        if(size.x < 0)
        {
            size.x = std::abs(size.x);
            pos.x -= size.x;
        }

        if(size.y < 0)
        {
            size.y = std::abs(size.y);
            pos.y -= size.y;
        }

        if(map.getSize().x <= pos.x + size.x || map.getSize().y <= pos.y + size.y)
            return false;

        if(pos.x < 0 || pos.y < 0)
            return false;

        for(int x = 0; x < size.x; x++)
        {
            for(int y = 0; y < size.y; y++)
            {
                if(map.getTile({x + pos.x, y + pos.y}))
                    return false;
            }
        }

        return true;
    };

    // Pool of pregenerated room size
    std::vector<sf::Vector2i> roomSizePool;
    for(int x = 0; x < dungeon.roomPoolSize; x++)
        roomSizePool.emplace_back(roomSizeDist(rng), roomSizeDist(rng));

    std::vector<Dungeon::Room>& placedRoom = dungeon.rooms;

    // get the size for the first room
    auto firstRoom = roomSizePool.back();
    roomSizePool.pop_back();

    // insert the first room on the middle of the map and in the placed room list
    placeRoom(map.getSize()/2, firstRoom);
    placedRoom.push_back({map.getSize()/2, firstRoom});

    while(!roomSizePool.empty())
    {
        const auto originDir = rnd(Dungeon::Room::UP, Dungeon::Room::RIGHT);
        auto dir = originDir;

        auto room = roomSizePool.back();
        roomSizePool.pop_back();

        do
        {
            std::vector<sf::Vector2i> possiblePos;

            const int offsetInt = dungeon.minimalDirectionalRoomDistance;
            const int minimalDist = dungeon.minimalRoomDistance;

            sf::Vector2i posDiff;
            sf::Vector2i offset;
            sf::Vector2i sideOffset;
            bool swapX = false;

            if(dir == Dungeon::Room::UP) // up
            {
                posDiff = {0, 0};
                offset = {0, offsetInt};
                sideOffset = {1, 0};
            }
            else if(dir == Dungeon::Room::LEFT) // left
            {
                posDiff = {0, 0};
                offset = {offsetInt, 0};
                sideOffset = {0, 1};
                swapX = true;
            }
            else if(dir == Dungeon::Room::DOWN) // dowindown
            {
                posDiff = {0, map.getSize().y - 1};
                offset = {0, -offsetInt};
                sideOffset = {1, 0};
            }
            else if(dir == Dungeon::Room::RIGHT) // right
            {
                posDiff = {map.getSize().x - 1, 0};
                offset = {-offsetInt, 0};
                sideOffset = {0, 1};
                swapX = true;
            }

            dir++;
            if(dir == Dungeon::Room::RIGHT + 1)
                dir = Dungeon::Room::UP;

            for(int y = 0; y < map.getSize().y; y++)
            {
                for(int x = 0; x < map.getSize().x; x++)
                {
                    sf::Vector2i pos = {x, y};

                    if(swapX)
                        std::swap(pos.x, pos.y);

                    if(posDiff.x)
                        pos.x = posDiff.x - pos.x;
                    if(posDiff.y)
                        pos.y = posDiff.y - pos.y;

                    auto sightCheckSize = room;

                    if(swapX)
                    {
                        if(posDiff.x)
                            sightCheckSize.x = -pos.x;
                        else
                            sightCheckSize.x = map.getSize().x - pos.x - 1;
                    }
                    else
                    {
                        if(posDiff.y)
                            sightCheckSize.y = -pos.y;
                        else
                            sightCheckSize.y = map.getSize().y - pos.y - 1;
                    }

                    if(
                        canPlaceRoom(pos, room) &&
                        canPlaceRoom(pos + offset - sf::Vector2i(minimalDist, minimalDist), room + sf::Vector2i(minimalDist*2, minimalDist*2)) &&
                        canPlaceRoom(pos + offset, room) &&
                        canPlaceRoom(pos, sightCheckSize))
                        possiblePos.push_back(pos + offset);
                }

                //if windowe found some possible position then go to next step: choosing one
                if(!possiblePos.empty())
                    break;
            }

            if(!possiblePos.empty())
            {
                auto pos = possiblePos[rnd(0, possiblePos.size() - 1)];

                placeRoom(pos, room);
                placedRoom.push_back({pos, room});

                break;
            }

            continue;
        }
        while(dir != originDir);

        if(dir == originDir)
            break;
    }

    std::vector<sf::Vector2i> roomPos;
    for(const auto& room : placedRoom)
        roomPos.push_back(room.pos + room.size/2);

    auto edges = triangulate(roomPos);
    auto& minimumEdges = dungeon.edges;
    minimumEdges = minimumSpanningTree(edges);

    for(const auto& edge : minimumEdges)
        edges.erase(std::remove(edges.begin(), edges.end(), edge), edges.end());

    for(int x = 0; x < dungeon.additionalEdge && edges.size();  x++)
    {
        auto edge = edges[rnd(0, edges.size() - 1)];
        edges.erase(std::remove(edges.begin(), edges.end(), edge), edges.end());

        minimumEdges.push_back(edge);
    }

    std::vector<std::pair<int, int>> roomEdges;
    for(const auto& edge : minimumEdges)
    {
        roomEdges.emplace_back(
            std::find(roomPos.begin(), roomPos.end(), edge.p1) - roomPos.begin(),
            std::find(roomPos.begin(), roomPos.end(), edge.p2) - roomPos.begin()
        );
    }

    int minDistToBorder = dungeon.minDoorDistToCorner;

    for(auto edge : roomEdges)
    {
        if(rnd(0, 1))
            std::swap(edge.first, edge.second);

        auto& r1 = placedRoom[edge.first];
        auto& r2 = placedRoom[edge.second];

        if(r1.pos.x + r1.size.x > r2.pos.x && r2.pos.x + r2.size.x > r1.pos.x)
        {
            auto min = std::max(r1.pos.x, r2.pos.x);
            auto max = std::min(r1.pos.x + r1.size.x, r2.pos.x + r2.size.x) - 1;

            auto pos = max;

            min += minDistToBorder;
            max -= minDistToBorder;

            if(min <= max)
            {
                pos = rnd(min, max);

                placeRoom(sf::Vector2i(pos, r1.pos.y), sf::Vector2i(1, r2.pos.y - r1.pos.y));
                dungeon.corridors.push_back({sf::Vector2i(pos, r1.pos.y + r1.size.y * (r1.pos.y < r2.pos.y)), sf::Vector2i(pos + 1, r2.pos.y + r2.size.y * !(r1.pos.y < r2.pos.y))});

                continue;
            }
        }
        else if(r1.pos.y + r1.size.y > r2.pos.y && r2.pos.y + r2.size.y > r1.pos.y)
        {
            auto min = std::max(r1.pos.y, r2.pos.y);
            auto max = std::min(r1.pos.y + r1.size.y, r2.pos.y + r2.size.y) - 1;

            auto pos = max;

            min += minDistToBorder;
            max -= minDistToBorder;

            if(min <= max)
            {
                pos = rnd(min, max);

                placeRoom(sf::Vector2i(r1.pos.x, pos), sf::Vector2i(r2.pos.x - r1.pos.x, 1));
                dungeon.corridors.push_back({sf::Vector2i(r1.pos.x + r1.size.x * (r1.pos.x < r2.pos.x), pos), sf::Vector2i(r2.pos.x + r2.size.x * (r2.pos.x < r1.pos.x), pos + 1)});

                continue;
            }
        }

        sf::Vector2i start = {};
        sf::Vector2i end = {};
        Dungeon::Room::Side side;

        if(r1.pos.x > r2.pos.x)
        {
            start.x = r1.pos.x;
            side = Dungeon::Room::LEFT;
        }
        else
        {
            start.x = r1.pos.x + r1.size.x;
            side = Dungeon::Room::RIGHT;
        }

        if(start.y == r1.doors[side].y)
            r1.doors[side].y = r1.pos.y + rnd(minDistToBorder, r1.size.y - minDistToBorder * 2);

        start.y = r1.doors[side].y;

        if(r2.pos.y > r1.pos.y)
        {
            end.y = r2.pos.y;
            side = Dungeon::Room::UP;
        }
        else
        {
            end.y = r2.pos.y + r2.size.y;
            side = Dungeon::Room::DOWN;
        }

        if(r2.doors[side].x == 0)
            r2.doors[side].x = r2.pos.x + rnd(minDistToBorder, r2.size.x - minDistToBorder * 2);

        end.x = r2.doors[side].x;

        sf::Vector2i corridor1 = {end.x - start.x, 1};
        sf::Vector2i corridor2 = {1, start.y - end.y};

        if(corridor2.y > 0)
            corridor2.y++;

        placeRoom(start, corridor1);
        placeRoom(end, corridor2);

        dungeon.corridors.push_back({start, start + corridor1});
        dungeon.corridors.push_back({end, end + corridor2});
    }
//    exit(0);
}
