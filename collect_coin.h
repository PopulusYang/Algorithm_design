#ifndef COLLECT_COIN_H
#define COLLECT_COIN_H
#include <vector>

enum class MAZE
{
    START,
    BOSS,
    TRAP,
    WALL,
    WAY,
    EXIT,
    SOURCE,
    LOCKER
};

typedef struct point{
    int x;
    int y;
} point;

class coin_collecter
{
private:
    MAZE **informations;
    std::vector<point> path;

public:
    coin_collecter(MAZE **informations):informations(informations){}
    std::vector<point> get_path(MAZE **newinformation = nullptr);
    void wait() {};
};

#endif