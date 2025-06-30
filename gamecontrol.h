#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include "mapbuild.h" // 包含地图生成器
#include "dp.h"

#include<QString>
#include<QPointF>
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

    GameController(gamemain* informations):
        gamemain(informations->mazesize),
        MazeGenerator(informations->mazesize),
        dp(informations->mazesize)
    {
        this->start = informations->start;
        this->end = informations->end;
        this->is_near_locker = informations->is_near_locker;
        this->bosshp = informations->bosshp;
        this->Skills = informations->Skills;
        this->dimension = informations->dimension;
        this->sourse = informations->sourse;
        this->sourse_value = informations->sourse_value;
        this->traps = informations->traps;
        this->received_clue = informations->received_clue;
        this->boss_in_map = informations->boss_in_map;
        for(int i = 0; i < MAXSIZE; ++i)//复制地图信息
        {
            for (int j = 0; j < MAXSIZE; ++j)
            {
                maze[i][j] = informations->maze[i][j];
            }
        }
    }

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
