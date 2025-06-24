#ifndef GAMEMAIN_H
#define GAMEMAIN_H

#include <unordered_set>
#include <unordered_map>

#define MAXSIZE 51 // 最大支持51x51的迷宫


enum class MAZE
{
    START,
    BOSS,
    TRAP,
    WALL,
    WAY,
    EXIT,
    SOURCE,
    LOCKER,
    CLUE
};

class point
{
public:
    int x;
    int y;

    point():x(0),y(0){}
    point(int x, int y):x(x),y(y){}
    bool operator==(const point &other) const
    {
        return x == other.x && y == other.y;
    }

};

// 为 point 特化 std::hash
namespace std
{
    template <>
    struct hash<point>
    {
        size_t operator()(const point &p) const
        {
            // 一个简单的哈希组合方法
            return hash<int>()(p.x) ^ (hash<int>()(p.y) << 1);
        }
    };
}

class gamemain
{
public:
    gamemain(int size):mazesize(size){}
    // gamemain() = default;
    int mazesize;
    point start;
    point end;

    std::unordered_set<point> sourse;

    int getSize()
    {
        return mazesize;
    }

    // 获取迷宫数组
    const int (*getMaze() const)[MAXSIZE] 
    {
        return maze;
    }

    bool inBounds(int x, int y)//判断是否越界
    {
        return x > 0 && y > 0 && x < mazesize && y < mazesize;
    }

protected:
    int dimension;
    int maze[MAXSIZE][MAXSIZE];
};

#endif