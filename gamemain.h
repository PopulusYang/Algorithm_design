#ifndef GAMEMAIN_H
#define GAMEMAIN_H

#include <unordered_set>
#include <unordered_map>
#include <vector>

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
struct Skill
{
    int id;
    int damage;
    int cooldown;
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
    bool operator!=(const point&other)const
    {
        return x!=other.x||y!=other.y;
    }

};


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
    gamemain(){}
    // gamemain() = default;
    int mazesize;
    bool is_near_locker;
    struct clue_content{
        point clue_position;
        int gen_order_index;      
        int password_dig_val;
    };
    std::vector<clue_content> received_clue;
    point start;
    point end;

    bool boss_in_map = false; // 是否有BOSS
    
    std::unordered_set<point> sourse;
    std::unordered_map<point, int> sourse_value; // 资源价值
    std::unordered_map<point, bool> traps;
    std::vector<Skill>Skills;//玩家的招式伤害和对应的冷却回合
    std::vector<int>bosshp;
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
    int maze[MAXSIZE][MAXSIZE];
    int dimension;

};

#endif
