#ifndef DP_H
#define DP_H

#include <cmath>
#include <vector>
#include <queue>
#include <iostream>

#include "gamemain.h"


struct State
{
    point pos;//该状态坐标
    int totalWeight;//该状态权值和
    std::vector<point> path;//该状态得到的路径

    bool operator<(const State &other) const
    {
        return totalWeight < other.totalWeight; // max-heap
    }
};

class dp : virtual public gamemain
{
private:
    int weight(point dest, point current) const;
    int getCellWeight(MAZE cellType) const;
    void printDPTable(const std::vector<std::vector<int>> &maxWeight) const;
public : 
    explicit dp(int size):gamemain(size){}

    std::vector<point> findBestPath();

};

#endif // DP_H