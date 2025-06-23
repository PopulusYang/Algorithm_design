#ifndef MAPBUILD_H
#define MAPBUILD_H

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <algorithm>
#include <utility>

#include "gamemain.h"

// 为了符合题目要求，定义一个固定的MAXSIZE，但内部使用动态的std::vector<std::string>
// 这样做更灵活且是C++的最佳实践。

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

class MazeGenerator :virtual public gamemain
{
public:
    
    // 构造函数，初始化迷宫尺寸和随机数生成器
    MazeGenerator(int size): gamemain(size){
        // 确保迷宫尺寸是奇数，这对算法至关重要
        if (size % 2 == 0) {
            size++;
        }
        // 确保尺寸不小于最小限制7
        if (size < 7) {
            size = 7;
        }
        if (size >= MAXSIZE) {
            size = MAXSIZE - 2;
             if (size % 2 == 0) {
                size++;
            }
        }

        this->dimension = size;
        this->rng.seed(std::time(nullptr)); // 使用当前时间作为随机数种子

        // 初始化迷宫网格，四周是墙(WALL)，内部是通路(WAY)
        for (int i = 0; i < MAXSIZE; ++i) {
            for (int j = 0; j < MAXSIZE; ++j) {
                maze[i][j] = static_cast<int>(MAZE::WAY);
            }
        }
        for (int i = 0; i < dimension; ++i) {
            maze[0][i] = static_cast<int>(MAZE::WALL);
            maze[dimension - 1][i] = static_cast<int>(MAZE::WALL);
            maze[i][0] = static_cast<int>(MAZE::WALL);
            maze[i][dimension - 1] = static_cast<int>(MAZE::WALL);
        }
    }

    // 生成迷宫主函数
    void generate() {
        divide(1, 1, dimension - 2, dimension - 2);
    }

    // 放置各种物件
    void placeFeatures() {
        int gold_count = 0;
        int trap_count = 0;
        int locker_count = 0;
        bool has_boss = false;

        if (dimension >= 15) {
            gold_count = 3;
            trap_count = 2;
            locker_count = 2;
            has_boss = true;
        } else if (dimension >= 7) {
            gold_count = 1;
            trap_count = 1;
            locker_count = 1;
        }

        std::vector<std::pair<int, int>> empty_cells;
        for (int r = 1; r < dimension - 1; ++r) {
            for (int c = 1; c < dimension - 1; ++c) {
                if (maze[r][c] == static_cast<int>(MAZE::WAY)) {
                    empty_cells.push_back({r, c});
                }
            }
        }
        std::shuffle(empty_cells.begin(), empty_cells.end(), rng);
        placeFeature(empty_cells, MAZE::START);
        placeFeature(empty_cells, MAZE::EXIT);
        if (has_boss) {
            placeFeature(empty_cells, MAZE::BOSS);
        }
        for (int i = 0; i < gold_count; ++i) {
            placeFeature(empty_cells, MAZE::SOURCE);
        }
        for (int i = 0; i < trap_count; ++i) {
            placeFeature(empty_cells, MAZE::TRAP);
        }
        for (int i = 0; i < locker_count; ++i) {
            placeFeature(empty_cells, MAZE::LOCKER);
        }
    }

    // 打印迷宫到控制台
    void print() const {
        for (int i = 0; i < dimension; ++i) {
            for (int j = 0; j < dimension; ++j) {
                char to_print;
                switch (static_cast<MAZE>(maze[i][j])) {
                    case MAZE::START: to_print = 'S'; break;
                    case MAZE::EXIT: to_print = 'E'; break;
                    case MAZE::WALL: to_print = '#'; break;
                    case MAZE::WAY: to_print = ' '; break;
                    case MAZE::SOURCE: to_print = 'G'; break;
                    case MAZE::TRAP: to_print = 'T'; break;
                    case MAZE::LOCKER: to_print = 'L'; break;
                    case MAZE::BOSS: to_print = 'B'; break;
                    default: to_print = '?'; break;
                }
                std::cout << to_print;
            }
            std::cout << std::endl;
        }
    }

    void print_num() const {
        for (int i = 0; i < dimension; ++i) {
            for (int j = 0; j < dimension; ++j) {
                std::cout << maze[i][j];
            }
            std::cout << std::endl;
        }
    }

    // 如果需要将结果存入 std::string maze[MAXSIZE][MAXSIZE]，可以调用此函数
    void exportToLegacyArray(std::string arr[MAXSIZE][MAXSIZE]) const {
        for(int r = 0; r < dimension; ++r) {
            for (int c = 0; c < dimension; ++c) {
                char ch;
                switch (static_cast<MAZE>(maze[r][c])) {
                    case MAZE::START: ch = 'S'; break;
                    case MAZE::EXIT: ch = 'E'; break;
                    case MAZE::WALL: ch = '#'; break;
                    case MAZE::WAY: ch = ' '; break;
                    case MAZE::SOURCE: ch = 'G'; break;
                    case MAZE::TRAP: ch = 'T'; break;
                    case MAZE::LOCKER: ch = 'L'; break;
                    case MAZE::BOSS: ch = 'B'; break;
                    default: ch = '?'; break;
                }
                arr[r][c] = ch;
            }
        }
    }

    

private:
    
    std::mt19937 rng; // Mersenne Twister 随机数引擎

    // 分治法核心递归函数
    void divide(int r, int c, int h, int w) {
        // 基准情况：如果区域太小，无法再分割，则返回
        if (h < 3 || w < 3) {
            return;
        }

        // 决定分割方向：如果宽度大于高度，则垂直分割，否则水平分割
        bool horizontal = (h > w);
        if (h == w) { // 如果是正方形，随机选择方向
            std::uniform_int_distribution<int> dist(0, 1);
            horizontal = dist(rng) == 0;
        }

        if (horizontal) {
            // 水平分割
            // 1. 选择一个偶数行来建造墙壁
            std::uniform_int_distribution<int> wall_dist(r + 1, r + h - 2);
            int wall_r = wall_dist(rng);
            if (wall_r % 2 != 0) wall_r++;
            if (wall_r >= r + h -1) wall_r -= 2;

            // 2. 选择一个奇数行来打开通道
            std::uniform_int_distribution<int> passage_dist(c, c + w - 1);
            int passage_c = passage_dist(rng);
            if (passage_c % 2 == 0) passage_c++; // 确保是奇数列
            if (passage_c >= c + w) passage_c -= 2;

            // 3. 建造墙壁并打开通道
            for (int i = c; i < c + w; ++i) {
                if (i != passage_c) {
                    maze[wall_r][i] = static_cast<int>(MAZE::WALL);
                }
            }

            // 4. 递归处理上下两个子区域
            divide(r, c, wall_r - r, w);
            divide(wall_r + 1, c, r + h - (wall_r + 1), w);

        } else {
            // 垂直分割
            // 1. 选择一个偶数列来建造墙壁
            std::uniform_int_distribution<int> wall_dist(c + 1, c + w - 2);
            int wall_c = wall_dist(rng);
            if (wall_c % 2 != 0) wall_c++;
            if (wall_c >= c + w - 1) wall_c -= 2;

            // 2. 选择一个奇数行来打开通道
            std::uniform_int_distribution<int> passage_dist(r, r + h - 1);
            int passage_r = passage_dist(rng);
            if (passage_r % 2 == 0) passage_r++;
            if (passage_r >= r + h) passage_r -= 2;

            // 3. 建造墙壁并打开通道
            for (int i = r; i < r + h; ++i) {
                if (i != passage_r) {
                    maze[i][wall_c] = static_cast<int>(MAZE::WALL);
                }
            }
            // 4. 递归处理左右两个子区域
            divide(r, c, h, wall_c - c);
            divide(r, wall_c + 1, h, c + w - (wall_c + 1));
        }
    }
    void placeFeature(std::vector<std::pair<int, int>>& cells, MAZE feature) {
        if (cells.empty()) return;
        std::pair<int, int> pos = cells.back();
        cells.pop_back();
        maze[pos.first][pos.second] = static_cast<int>(feature);
    }
};

#endif
