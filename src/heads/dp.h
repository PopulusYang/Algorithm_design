#ifndef DP_H
#define DP_H

#include <cmath>
#include <vector>
#include <queue>
#include <iostream>
#include<stack>
#include <algorithm>
#include "gamemain.h"
#include"collect_coin.h"

struct djstruct
{
    int lenght = 0;
    std::vector<point> path;
};

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
    void isWorth(djstruct& input);
    void full_the_path(std::vector<point> &input);
    std::pair<std::unordered_set<point>, point> greedy_simulate(std::vector<point> &path, size_t start_idx);

public:
    explicit dp(int size) : gamemain(size), collecter(this)
    {}
    coin_collecter collecter;
    djstruct Dijkstra(point S, point E);
    std::vector<point> findBestPath(point playerstart);
    std::vector<point> simulate(point playerstart);
};

#endif // DP_H
