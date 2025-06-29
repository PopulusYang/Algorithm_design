#ifndef MAPBUILD_H
#define MAPBUILD_H

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <algorithm>
#include <utility>
#include <windows.h> // 包含windows.h以使用SetConsoleOutputCP

#include "gamemain.h"

// ????????????????MAXSIZE?????????std::vector<std::string>
// ????????C++??????

class MazeGenerator : virtual public gamemain
{
public:
    int gen_order = 0;
    struct Division
    {
        int r, c, h, w;
    };
    std::vector<Division> division_stack;

    std::pair<int, int> start_m; // 起点坐标
    std::pair<int, int> exit;    // 终点坐标
    std::pair<int, int> boss;    // BOSS坐标
    std::pair<int, int> locker;  // 机关坐标
    std::pair<int, int> clue;    // 线索坐标

    // 线索内容，到达线索对应的坐标时，利用坐标作为参数，读取这个线索指示出的密码位和密码值（例如密码：456，访问这个线索之后，可以知道4是密码值，1是密码位，即密码的第一位是4）
    std::unordered_map<point, clue_content> clue_set;
    clue_content clue_arr[4];
    // 构造函数，初始化迷宫尺寸和随机数生成器
    MazeGenerator(int size) : gamemain(size)
    {
        mazesize = size;
        // ??????????????????
        if (size % 2 == 0)
        {
            size++;
        }
        // ???????????7
        if (size < 7)
        {
            size = 7;
        }
        if (size >= MAXSIZE)
        {
            size = MAXSIZE - 2;
            if (size % 2 == 0)
            {
                size++;
            }
        }

        this->dimension = size;
        this->rng.seed(std::time(nullptr)); // ?????????????

        // ????????????(WALL)??????(WAY)
        for (int i = 0; i < MAXSIZE; ++i)
        {
            for (int j = 0; j < MAXSIZE; ++j)
            {
                maze[i][j] = static_cast<int>(MAZE::WAY);
            }
        }
        for (int i = 0; i < dimension; ++i)
        {
            maze[0][i] = static_cast<int>(MAZE::WALL);
            maze[dimension - 1][i] = static_cast<int>(MAZE::WALL);
            maze[i][0] = static_cast<int>(MAZE::WALL);
            maze[i][dimension - 1] = static_cast<int>(MAZE::WALL);
        }
    }

    // ???????
    void generate()
    {
        generate_init();
        while (generateStep())
            ;
    }

    void generate_init()
    {
        division_stack.clear();
        division_stack.push_back({1, 1, dimension - 2, dimension - 2});
    }

    bool generateStep()
    {
        if (division_stack.empty())
        {
            return false; // 生成完成
        }

        Division current = division_stack.back();
        division_stack.pop_back();

        int r = current.r;
        int c = current.c;
        int h = current.h;
        int w = current.w;

        if (h < 3 || w < 3)
        {
            return !division_stack.empty();
        }

        bool horizontal = (h > w);
        if (h == w)
        {
            std::uniform_int_distribution<int> dist(0, 1);
            horizontal = dist(rng) == 0;
        }

        if (horizontal)
        {
            std::uniform_int_distribution<int> wall_dist(r + 1, r + h - 2);
            int wall_r = wall_dist(rng);
            if (wall_r % 2 != 0)
                wall_r++;
            if (wall_r >= r + h - 1)
                wall_r -= 2;

            std::uniform_int_distribution<int> passage_dist(c, c + w - 1);
            int passage_c = passage_dist(rng);
            if (passage_c % 2 == 0)
                passage_c++;
            if (passage_c >= c + w)
                passage_c -= 2;

            for (int i = c; i < c + w; ++i)
            {
                if (i != passage_c)
                {
                    maze[wall_r][i] = static_cast<int>(MAZE::WALL);
                }
            }

            division_stack.push_back({r, c, wall_r - r, w});
            division_stack.push_back({wall_r + 1, c, r + h - (wall_r + 1), w});
        }
        else // Vertical
        {
            std::uniform_int_distribution<int> wall_dist(c + 1, c + w - 2);
            int wall_c = wall_dist(rng);
            if (wall_c % 2 != 0)
                wall_c++;
            if (wall_c >= c + w - 1)
                wall_c -= 2;

            std::uniform_int_distribution<int> passage_dist(r, r + h - 1);
            int passage_r = passage_dist(rng);
            if (passage_r % 2 == 0)
                passage_r++;
            if (passage_r >= r + h)
                passage_r -= 2;

            for (int i = r; i < r + h; ++i)
            {
                if (i != passage_r)
                {
                    maze[i][wall_c] = static_cast<int>(MAZE::WALL);
                }
            }

            division_stack.push_back({r, c, h, wall_c - c});
            division_stack.push_back({r, wall_c + 1, h, c + w - (wall_c + 1)});
        }
        return true;
    }

    // ????????
    void placeFeatures()
    {
        int gold_count = 0;
        int trap_count = 0;
        int locker_count = 0;
        int clue_count = 3;

        gold_count = 4 * dimension - 24;
        trap_count = dimension - 6;

        // 为 gold_count 添加一个上限，防止DP算法因资源过多而崩溃
        if (gold_count > 14)
        {
            gold_count = 14;//避免金币数量太多导致dp崩溃
        }
        if(trap_count > 20)
        {
            trap_count = 20;
        }

        if (gold_count < 0)
            gold_count = 0;
        if (trap_count < 0)
            trap_count = 0;

        // 仅在较大的地图上放置BOSS和机关
        if (dimension >= 9)
        {
            locker_count = 1;
        }
        else if (dimension >= 7)
        {
            locker_count = 1;
        }
        start = {1, 1};
        maze[start.x][start.y] = static_cast<int>(MAZE::START);
        end = {dimension - 2, dimension - 2};
        maze[end.x][end.y] = static_cast<int>(MAZE::EXIT);

        std::vector<std::pair<int, int>> empty_cells;
        for (int r = 1; r < dimension - 1; ++r)
        {
            for (int c = 1; c < dimension - 1; ++c)
            {
                if (maze[r][c] == static_cast<int>(MAZE::WAY))
                {
                    empty_cells.push_back({r, c});
                }
            }
        }
        std::shuffle(empty_cells.begin(), empty_cells.end(), rng);

        for (int i = 0; i < gold_count; ++i)
        {
            placeFeature(empty_cells, MAZE::SOURCE);
        }
        for (int i = 0; i < trap_count; ++i)
        {
            placeFeature(empty_cells, MAZE::TRAP);
        }
        for (int i = 0; i < locker_count; ++i)
        {
            placeFeature(empty_cells, MAZE::LOCKER);
        }
        for (int i = 0; i < clue_count; ++i)
        {
            placeFeature(empty_cells, MAZE::CLUE);
        }
    }

    void print() const
    {
        for (int i = 0; i < dimension; ++i)
        {
            for (int j = 0; j < dimension; ++j)
            {
                char to_print;
                switch (static_cast<MAZE>(maze[i][j]))
                {
                case MAZE::START:
                    to_print = 'S';
                    break;
                case MAZE::EXIT:
                    to_print = 'E';
                    break;
                case MAZE::WALL:
                    to_print = '#';
                    break;
                case MAZE::WAY:
                    to_print = ' ';
                    break;
                case MAZE::SOURCE:
                    to_print = 'G';
                    break;
                case MAZE::TRAP:
                    to_print = 'T';
                    break;
                case MAZE::LOCKER:
                    to_print = 'L';
                    break;
                case MAZE::BOSS:
                    to_print = 'B';
                    break;
                case MAZE::CLUE:
                    to_print = 'C';
                    break;
                default:
                    to_print = '?';
                    break;
                }
                std::cout << to_print;
            }
            std::cout << std::endl;
        }
    }

    void print_num() const
    {
        for (int i = 0; i < dimension; ++i)
        {
            for (int j = 0; j < dimension; ++j)
            {
                std::cout << maze[i][j];
            }
            std::cout << std::endl;
        }
    }

    // ????????? std::string maze[MAXSIZE][MAXSIZE]????????
    void exportToLegacyArray(std::string arr[MAXSIZE][MAXSIZE]) const
    {
        for (int r = 0; r < dimension; ++r)
        {
            for (int c = 0; c < dimension; ++c)
            {
                char ch;
                switch (static_cast<MAZE>(maze[r][c]))
                {
                case MAZE::START:
                    ch = 'S';
                    break;
                case MAZE::EXIT:
                    ch = 'E';
                    break;
                case MAZE::WALL:
                    ch = '#';
                    break;
                case MAZE::WAY:
                    ch = ' ';
                    break;
                case MAZE::SOURCE:
                    ch = 'G';
                    break;
                case MAZE::TRAP:
                    ch = 'T';
                    break;
                case MAZE::LOCKER:
                    ch = 'L';
                    break;
                case MAZE::BOSS:
                    ch = 'B';
                    break;
                case MAZE::CLUE:
                    ch = 'C';
                    break;
                default:
                    ch = '?';
                    break;
                }
                arr[r][c] = ch;
            }
        }
    }

    // ??????
    int getsize()
    {
        return mazesize;
    }

    //??????
    int (*getmaze())[MAXSIZE]
    {
        return maze;
    }

    // ??????
    std::pair<int, int> getStart() const
    {
        return start_m;
    }

    // ??????
    std::pair<int, int> getExit() const
    {
        return exit;
    }

    // ??BOSS??
    std::pair<int, int> getBoss() const
    {
        return boss;
    }

    // ??????
    std::pair<int, int> getLocker() const
    {
        return locker;
    }

    // ??????
    std::pair<int, int> getClue() const
    {
        return clue;
    }

    // 生成完地图之后，利用这个函数就可以得到密码锁密码的值
    int getpassword() const
    {
        int password = 0;
        for (int i = 1; i <= 3; i++)
        {
            int temp;
            temp = clue_arr[i].password_dig_val;
            password *= 10;
            password += temp;
        }
        return password;
    }

    // 遇到线索时，调用这两个函数获取线索，集齐三个线索，就可以得到密码的值，与getpassword()的返回值比较，即可判断密码是否正确
    int getclue_index(point clue_point)
    {
        auto it = clue_set.find(clue_point);
        if (it != clue_set.end())
        {
            return it->second.gen_order_index;
        }
        return -1; // 或者其他适当的错误值
    }

    int getclue_val(point clue_point)
    {
        auto it = clue_set.find(clue_point);
        if (it != clue_set.end())
        {
            return it->second.password_dig_val;
        }
        return -1; // 或者其他适当的错误值
    }

private:
    std::mt19937 rng; // Mersenne Twister ?????


    void placeFeature(std::vector<std::pair<int, int>> &cells, MAZE feature)
    {
        if (cells.empty())
            return;
        std::pair<int, int> pos = cells.back();
        cells.pop_back();
        if (feature == MAZE::START)
            start = {pos.first, pos.second};
        else if (feature == MAZE::EXIT)
            end = {pos.first, pos.second};
        maze[pos.first][pos.second] = static_cast<int>(feature);
        switch (feature)
        {
        case MAZE::START:
            start_m = pos;
            break;
        case MAZE::EXIT:
            exit = pos;
            break;
        case MAZE::BOSS:
            boss = pos;
            break;
        case MAZE::LOCKER:
            locker = pos;
            break;
        case MAZE::TRAP:
            traps.insert({point(pos.first, pos.second), false});
            break;
        case MAZE::CLUE:
        {
            gen_order++;
            point temp_point;
            clue_content temp_content;
            temp_point.x = pos.first;
            temp_point.y = pos.second;
            temp_content.gen_order_index = gen_order;
            temp_content.password_dig_val = (pos.first * pos.second * 0xBEEF) % 10;
            temp_content.clue_position.x = pos.first;
            temp_content.clue_position.y = pos.second;
            clue_arr[gen_order] = temp_content;
            clue_set.insert({temp_point, temp_content});
            std::cout << "insert password to hash: " << temp_content.gen_order_index << std::endl;
            temp_point.x = pos.first;
            temp_point.y = pos.second;
            clue = pos;
            break;
        }
        case MAZE::SOURCE:
        {
            point temp_point;
            temp_point.x = pos.first;
            temp_point.y = pos.second;
            std::uniform_int_distribution<int> ranvalue(0, 100);
            int val = ranvalue(rng);
            sourse_value.insert({temp_point, val});
            sourse.insert(temp_point);
            break;
        }
        default:
            break;
        }
    }
};



#endif
