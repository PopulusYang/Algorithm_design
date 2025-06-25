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
    bool collected;
    int valud;
}goldcoin;

class coin_collecter:virtual public gamemain
{   
public:
    coin_collecter():gamemain(){};
    ~coin_collecter(){};
   point findway(point currpoint);
   unordered_map<point,int>sourse_value;
   array<goldcoin,8>view;
   point nextpoint;
};

#endif
