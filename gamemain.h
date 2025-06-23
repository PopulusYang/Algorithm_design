#ifndef GAMEMAIN_H
#define GAMEMAIN_H

#define MAXSIZE 51 // 最大支持51x51的迷宫

class gamemain
{
public:
    gamemain(int size):mazesize(size){}
    gamemain() = default;
    int mazesize;
    int getSize()
    {
        return mazesize;
    }

    // 获取迷宫数组
    const int (*getMaze() const)[MAXSIZE] 
    {
        return maze;
    }

protected:
    int dimension;
    int maze[MAXSIZE][MAXSIZE];
};

#endif