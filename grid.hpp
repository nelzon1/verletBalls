#pragma once

#include <vector>

using namespace std;


struct Cell
{
    uint32_t x;
    uint32_t y;
    vector<uint32_t> cellObjects;

    Cell() = default;

    Cell(uint32_t xpos, uint32_t ypos)
    : x{xpos}
    , y{ypos}
    , cellObjects{}
    {}

    Cell(uint32_t xpos, uint32_t ypos, vector<uint32_t> objects)
    : x{xpos}
    , y{ypos}
    , cellObjects{objects}
    {}

    vector<uint32_t> getObjects()
    {
        return cellObjects;
    }

    void addObject(uint32_t objectIndex) {
        cellObjects.emplace_back(objectIndex);
    }

    void clearObjects(){
        cellObjects.clear();
    }

};

struct Grid
{
    uint32_t height;
    uint32_t width;
    vector<vector<Cell>> grid;

    Grid() = default;

    Grid(uint32_t height, uint32_t width)
    : height{height}
    , width{width}
    {
        for (uint32_t i = 0; i < height; ++i){
            grid.emplace_back(vector<Cell>());
            for (uint32_t j = 0; j < width; ++j){
                grid[i].emplace_back(Cell(i, j));
            }
        }
    }

    Cell& getCell(uint32_t x, uint32_t y) {
        return grid[x][y];
    }

};