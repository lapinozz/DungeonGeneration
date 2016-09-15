#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "tilemap.hpp"
#include "dungeonGenerator.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

int main()
{
    sf::RenderWindow window(sf::VideoMode(750, 750), "map");

    ImGui::SFML::Init(window);

    Dungeon dungeon;

    sf::Clock deltaClock;
    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();

        dungeon.rooms.clear();
        dungeon.corridors.clear();
        dungeon.edges.clear();

        ImGui::SFML::Update(deltaClock.restart());

        ImGui::Begin("Data");

        ImGui::SliderInt("Seed", &dungeon.seed, -1, 99999);

        ImGui::SliderInt("Minimum Room Size", &dungeon.roomSizeMin, 2, dungeon.roomSizeMax);
        ImGui::SliderInt("Maximum Room Size", &dungeon.roomSizeMax, dungeon.roomSizeMin, std::min(dungeon.size.x, dungeon.size.y));

        ImGui::SliderInt("Room Pool Size", &dungeon.roomPoolSize, 1, 100);

        ImGui::SliderInt("Distance", &dungeon.minimalRoomDistance, 1, 100);
        ImGui::SliderInt("Directional Distance", &dungeon.minimalDirectionalRoomDistance, 1, 100);

        ImGui::SliderInt("Additional Corridors", &dungeon.additionalEdge, 0, 100);

        ImGui::SliderInt("Distance from door to corner", &dungeon.minDoorDistToCorner, 0, dungeon.roomSizeMin - 2);

        ImGui::End(); // end windowindowindow

        generateDungeon(dungeon);

        sf::RectangleShape rect({10, 10});
        rect.setFillColor(sf::Color::White);
        for(const auto& room: dungeon.rooms)
        {
            rect.setPosition((sf::Vector2f)room.pos * 10.f);
            rect.setSize((sf::Vector2f)room.size * 10.f);
            window.draw(rect);
        }

        sf::CircleShape circle;
        circle.setRadius(3);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Blue);
        for(const auto& room : dungeon.rooms)
        {
            circle.setPosition((sf::Vector2f)(room.pos + room.size/2) * 10.f);
            circle.move(0.5f, 0.5f);
            window.draw(circle);
        }

        for(const auto& edge : dungeon.edges)
        {
            sf::VertexArray va;
            va.setPrimitiveType(sf::Lines);
            va.append({(sf::Vector2f)edge.p1*10.f, sf::Color::Yellow});
            va.append({(sf::Vector2f)edge.p2*10.f, sf::Color::Yellow});
            window.draw(va);
        }

        rect.setFillColor(sf::Color::Green);
        for(const auto& corridor : dungeon.corridors)
        {
            rect.setPosition((sf::Vector2f)corridor.start*10.f);
            rect.setSize((sf::Vector2f)corridor.end*10.f - (sf::Vector2f)corridor.start*10.f);

            window.draw(rect);
        }

        ImGui::Render();
        window.display();
    }
}
