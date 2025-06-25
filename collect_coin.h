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



class coin_collecter:virtual public gamemain
{   
public:
    coin_collecter() : gamemain(){}
    coin_collecter(int **map, int size);
    ~coin_collecter();
   bool ifsourvaild();
    point findway();
   void oneview(int index,point viewpoint);
    unordered_map<point, int> sourse_value;
    array<goldcoin, 8> view;
   // point nextpoint;
};

#endif
