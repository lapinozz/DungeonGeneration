#pragma once

#include <vector>

template <typename TileType>
struct TileMap
{
    void setSize(const sf::Vector2i& size)
    {
        tiles.resize(size.x * size.y);
        width = size.x;
    }

    sf::Vector2i getSize() const
    {
        return {width, static_cast<int>(tiles.size())/width};
    }

    void setTile(const sf::Vector2i& pos, const TileType tile)
    {
        tiles[pos.y * width + pos.x] = tile;
    }

    TileType getTile(const sf::Vector2i& pos) const
    {
        return tiles[pos.y * width + pos.x];
    }

    TileType& at(const sf::Vector2i& pos)
    {
        return tiles[pos.y * width + pos.x];
    }

    static sf::FloatRect tileToRect(const sf::Vector2i& pos, const sf::Vector2f& tileSize)
    {
        sf::FloatRect rect;

        rect.left = pos.x * tileSize.x;
        rect.top = pos.y * tileSize.y;
        rect.width = tileSize.x;
        rect.height = tileSize.y;

        return rect;
    }

    std::vector<TileType> tiles;

private:
    int width;
};
