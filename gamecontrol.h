#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include "mapbuild.h" // 包含地图生成器
#include "dp.h"

class GameController : public MazeGenerator, public dp
{
public:
    GameController(int size) : gamemain(size), MazeGenerator(size), dp(size) {}
};


#endif