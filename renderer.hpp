#pragma once
#include "solver.hpp"


class Renderer
{
public:
    explicit
    Renderer(sf::RenderTarget& target)
        : m_target{target}

    {

    }

    void render(const Solver& solver) const
    {
        // Render constraint
        const sf::Vector3f constraint = solver.getConstraint();
        sf::CircleShape constraint_background{constraint.z};
        constraint_background.setOrigin(constraint.z, constraint.z);
        constraint_background.setFillColor(sf::Color::Black);
        constraint_background.setPosition(constraint.x, constraint.y);
        constraint_background.setPointCount(128);
        m_target.draw(constraint_background);

        // Render objects
        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        const auto& objects = solver.getObjects();
        for (const auto& obj : objects) {
            circle.setPosition(obj.position);
            circle.setScale(obj.radius, obj.radius);
            circle.setFillColor(obj.color);
            m_target.draw(circle);
        }
    }

    void loadText(std::string fontPath)
    {
        if (!m_font.loadFromFile(fontPath))
        {
            // prbolems
        }
    }

    void drawText(std::string displayText) 
    {
        sf::Text text;
        text.setFont(m_font);
        text.setString(displayText);
        text.setCharacterSize(50);
        text.setFillColor(sf::Color::Red);
        text.setOrigin(1.0f, 1.0f);
        text.setPosition(20.0f, 20.0f);
        text.setStyle(sf::Text::Bold);
        m_target.draw(text);
    }

        void drawFPS(std::string displayText) 
    {
        sf::Text text;
        text.setFont(m_font);
        text.setString(displayText);
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::Black);
        text.setOrigin(1.0f, 1.0f);
        text.setPosition(20.0f, 80.0f);
        text.setStyle(sf::Text::Bold);
        m_target.draw(text);
    }

        void drawFrameTime(std::string displayText) 
    {
        sf::Text text;
        text.setFont(m_font);
        text.setString(displayText);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Blue);
        text.setOrigin(1.0f, 1.0f);
        text.setPosition(20.0f, 120.0f);
        text.setStyle(sf::Text::Bold);
        m_target.draw(text);
    }

private:
    sf::RenderTarget& m_target;
    sf::Font m_font;
};