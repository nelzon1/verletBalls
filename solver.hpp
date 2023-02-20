#pragma once
#include "SFML/Graphics.hpp"
#include <vector>
#include <cmath>
#include "grid.hpp"

#include "utils/math.hpp"


struct VerletObject
{
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float       radius    = 10.0f;
    sf::Color   color     = sf::Color::White;

    VerletObject() = default;
    VerletObject(sf::Vector2f position_, float radius_)
        : position{position_}
        , position_last{position_}
        , acceleration{0.0f, 0.0f}
        , radius{radius_}
    {}

    void update(float dt)
    {
        // Compute how much we need
        const sf::Vector2f displacement = position - position_last;

        // Update position
        position_last = position;
        position = position + displacement + acceleration * (dt * dt);

        // Reset acceleration
        acceleration = {};
    }

    void accelerate(sf::Vector2f a) 
    {
        acceleration += a;
    }

    void setVelocity(sf::Vector2f v, float dt)
    {
        position_last = position - (v * dt);
    }

    void addVelocity(sf::Vector2f v, float dt) 
    {
        position_last -= v * dt;
    }

    [[nodiscard]]
    sf::Vector2f getVelocity(float dt) const
    {
        return (position - position_last) / dt;
    }

};

class Solver
{
public:
    Solver() = default;

    VerletObject& addObject(sf::Vector2f position, float radius)
    {
        return m_objects.emplace_back(position, radius);
    }

    void update()
    {
        m_time += m_frame_dt;
        const float step_dt = getStepDt();
        for (uint32_t i{m_sub_steps}; i--;) {
            applyGravity();
            if ( m_objects.size() < 200 ) checkCollisions(step_dt);
            else checkCollisionsGrid(step_dt);
            applyConstraint();
            updateObjects(step_dt);
        }
    }

    void setSimulationUpdateRate(uint32_t rate)
    {
        m_frame_dt = 1.0f / static_cast<float>(rate);
    }

    void setConstraint(sf::Vector2f position, float radius)
    {
        m_constraint_center = position;
        m_constraint_radius = radius;
    }

    void setSubStepsCount(uint32_t sub_steps) 
    {
        m_sub_steps = sub_steps;
    }

    void setObjectVelocity(VerletObject& object, sf::Vector2f v)
    {
        object.setVelocity(v, getStepDt());
    }

    void initilaizeGrid(uint32_t height, uint32_t width) {
        grid = Grid(width, height);
    }

    void clearGrid() {
        for (uint32_t i = 0; i < grid.width; ++i) {
            for (uint32_t j = 0; j < grid.height; ++j) {
                grid.getCell(i,j).clearObjects();
            }
        }
    }

    void assignObjectsToGrid() {
        clearGrid();
        for (uint32_t k = 0; k < m_objects.size(); ++k) {
            VerletObject& obj = m_objects[k];
            uint32_t x_bin = obj.position.x / 100;
            uint32_t y_bin = obj.position.y / 100;
            grid.getCell(x_bin,y_bin).addObject(k);
        }
    }

    [[nodiscard]]
    const std::vector<VerletObject>& getObjects() const{
        return m_objects;
    }

    [[nodiscard]]
    sf::Vector3f getConstraint() const
    {
        return {m_constraint_center.x, m_constraint_center.y, m_constraint_radius};
    }

    [[nodiscard]]
    uint64_t getObjectsCount() const{
        return m_objects.size();
    }

    [[nodiscard]]
    float getTime() const
    {
        return m_time;
    }

    [[nodiscard]]
    float getStepDt() const
    {
        return m_frame_dt / static_cast<float>(m_sub_steps);
    }

private:
    uint32_t   m_sub_steps  = 1;
    sf::Vector2f   m_gravity = {0.0f, 1000.0f};
    sf::Vector2f   m_constraint_center;
    float m_constraint_radius = 100.0f;
    std::vector<VerletObject> m_objects;
    float m_time = 0.0f;
    float m_frame_dt = 0.0f;
    Grid grid;
    const float response_coef = 0.75f;

    void applyGravity()
    {
        for (auto& obj : m_objects) {
            obj.accelerate(m_gravity);
        }
    }

    void checkCollisions(float dt)
    {
        const uint64_t objects_count = m_objects.size();
        // Iterate on all objects
        for (uint64_t i{0}; i < objects_count; ++i) {
            VerletObject& object_1 = m_objects[i];
            // Iterate on object involevd in new collision pairs
            for (uint64_t k{i + 1}; k < objects_count; ++k) {
                VerletObject& object_2 = m_objects[k];
                collide(object_1, object_2);
            }
        }
    }

    void collide(VerletObject& obj_1, VerletObject& obj_2){
        const sf::Vector2f v = obj_1.position - obj_2.position;
        const float dist2 = v.x * v.x + v.y * v.y;
        const float min_dist = obj_1.radius + obj_2.radius;
        // Check overlap
        if (dist2 < min_dist * min_dist) {
            const float  dist = sqrt(dist2);
            const sf::Vector2f n = v / dist;
            const float mass_ratio_1 = obj_1.radius / (obj_1.radius + obj_2.radius);
            const float mass_ratio_2 = obj_2.radius / (obj_1.radius + obj_2.radius);
            const float delta = 0.5f * response_coef * (dist - min_dist);
            // Update positions
            obj_1.position -= n * (mass_ratio_2 * delta);
            obj_2.position += n * (mass_ratio_1 * delta);
        }
    }

    void checkCollisionsGrid(float dt) {
        assignObjectsToGrid();
        // Iterate over all squares except the outside boundary layer
        for (uint16_t x = 1; x < grid.width - 1; ++x) {
            for (uint16_t y = 1; y < grid.height - 1; ++y) {
                Cell& cell_1 = grid.getCell(x,y);
                // Check the adjacent squares
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        //Check 2 cells against each other
                        Cell& cell_2 = grid.getCell(x + dx, y + dy);
                        checkCollisionsBetweenCells(cell_1, cell_2);
                    }
                }
            }
        }
        collideBoundaries();
    }

    void collideBoundaries() {
        for (uint16_t x = 0; x < grid.width; x += grid.width - 1) {
            for (uint16_t y = 0; y < grid.height; ++y) {
                Cell& cell = grid.getCell(x,y);
                checkCollisionsBetweenCells(cell, cell);
            }
        }
        for (uint16_t y = 0; y < grid.height; y += grid.height - 1) {
            for (uint16_t x = 0; x < grid.width; ++x) {
                 Cell& cell = grid.getCell(x,y);
                     checkCollisionsBetweenCells(cell, cell);
            }
        }
    }

    void checkCollisionsBetweenCells(Cell& cell_1, Cell& cell_2)
    {
        if (cell_1.cellObjects.size() == 0 || cell_2.cellObjects.size() == 0) return;
        for ( uint16_t i = 0; i < cell_1.cellObjects.size(); ++i){
            uint32_t id_1 = cell_1.cellObjects[i];
            for ( uint16_t j = 0; j < cell_2.cellObjects.size(); ++j){
                uint32_t id_2 = cell_2.cellObjects[j];
                if (id_1 != id_2){
                    collide(m_objects[id_1], m_objects[id_2]);
                }
            }
        }
    }

    void applyConstraint()
    {
        for (auto& obj : m_objects) {
            const sf::Vector2f v = m_constraint_center - obj.position;
            const float dist = sqrt(v.x * v.x + v.y * v.y);
            if (dist > (m_constraint_radius - obj.radius)) {
                const sf::Vector2f n = v / dist;
                obj.position = m_constraint_center - n * (m_constraint_radius - obj.radius);
            }
        }
    }

    void updateObjects(float dt)
    {
        for (auto& obj : m_objects) {
            obj.update(dt);
        }
    }

};