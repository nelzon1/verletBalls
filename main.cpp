#include <iostream>
#include <SFML/Graphics.hpp>
#include "solver.hpp"
#include "renderer.hpp"
#include "grid.hpp"
#include "utils/number_generator.hpp"
#include "utils/math.hpp"


static sf::Color getRainbow(float t)
{
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * Math::PI);
    const float b = sin(t + 0.66f * 2.0f * Math::PI);
    return {static_cast<uint8_t>(255.0f * r * r),
            static_cast<uint8_t>(255.0f * g * g),
            static_cast<uint8_t>(255.0f * b * b)};
}

int32_t main(int32_t, char*[])
{
    // Create window
    constexpr int32_t window_width = 1000;
    constexpr int32_t window_height = 1000;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 3;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Verlet", sf::Style::Default, settings);
    const uint32_t frame_rate = 60;
    window.setFramerateLimit(frame_rate);

    Solver solver;
    Renderer renderer{window};
    renderer.loadText("arial.ttf");

    // Solver configuration
    solver.setConstraint({static_cast<float>(window_width) * 0.5f, static_cast<float>(window_height) * 0.5f}, 450.0f);    
    solver.setSubStepsCount(8);
    solver.setSimulationUpdateRate(frame_rate);
    

    // Set simulation attributes
    const float objects_spawn_delay = 0.02f;
    const float objects_spawn_speed = 1200.0f;
    const sf::Vector2f objects_spawn_position = {500.0f, 200.0f};
    const float object_min_radius = 1.0f;
    const float object_max_radius = 20.0f;
    const uint32_t max_objects_count  = 1200;
    const float max_angle = 1.0f;

    solver.initilaizeGrid(24);

    sf::Clock clock;
    sf::Clock frameClock;
    // Main loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        }

        if (solver.getObjectsCount() < max_objects_count && clock.getElapsedTime().asSeconds() >= objects_spawn_delay) {
            clock.restart();
            auto& object = solver.addObject(objects_spawn_position, RNGf::getRange(object_min_radius, object_max_radius));
            const float t = solver.getTime();
            const float angle = max_angle * sin(t) + Math::PI * 0.5f;
            solver.setObjectVelocity(object, objects_spawn_speed * sf::Vector2f{cos(angle), sin(angle)});
            object.color = getRainbow(t);
        }

        //solver.assignObjectsToGrid();
        solver.update();
        window.clear(sf::Color::White);
        renderer.render(solver);
        renderer.drawText("Objects: " + std::to_string((int) solver.getObjectsCount()));
        uint32_t frameTimeMs = frameClock.getElapsedTime().asMilliseconds();
        uint32_t FPS = 1000 / frameTimeMs;
        renderer.drawFPS("FPS: " + std::to_string(FPS));
        renderer.drawFrameTime("Frame Time: " + std::to_string(frameTimeMs) + "ms");
        frameClock.restart();
        window.display();

    }

    return 0;
}   