#ifndef COLLECT_COIN_H
#define COLLECT_COIN_H
#include <vector>
#include<iostream>
#include<unordered_map>
#include<array>
#include"gamemain.h"

using namespace std;
typedef struct goldcoin
{
    point coinpoint;
    bool accessible;//某个位置是否是可以去的
    int value;
    MAZE type;
    double valueff;//性价比
}goldcoin;


class coin_collecter
{   
public:
    coin_collecter(gamemain *informations) : informations(informations){}
    ~coin_collecter(){};
    gamemain *informations = nullptr;
    std::unordered_set<point> tempset;
    bool ifsourvaild(point currentpoint);
    point findway(point currentpoint);
    void oneview(int index,point viewpoint);
    unordered_map<point, int> sourse_value;
    array<goldcoin, 8> view;
    point currentpoint;
   // point nextpoint;
};

#endif
