#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include "mapbuild.h" // 包含地图生成器
#include "dp.h"


struct DamageIndicator
    {
        QString text;
        QPointF position;
        int lifetime;
    };

class GameController : public MazeGenerator, public dp
{
public:
    GameController(int size) : gamemain(size), MazeGenerator(size), dp(size) {}

    bool inBounds(int x, int y) // 判断是否越界
    {
        return x > 0 && y > 0 && x < mazesize && y < mazesize;
    }

    void setMazeCell(int x, int y, MAZE type)
    {
        if (inBounds(x, y))
        {
            maze[x][y] = static_cast<int>(type);
        }
    }

    
};

#endif