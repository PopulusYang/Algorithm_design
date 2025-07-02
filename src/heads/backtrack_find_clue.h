#ifndef BACKTRACK_FIND_CLUE_H
#define BACKTRACK_FIND_CLUE_H
#include "mapbuild.h" 
/*
*@brief 构造路径生成器对象后，使用回溯法生成从起始坐标到所有线索的路径
*@param 迷宫信息
*@param 起始坐标（从这个坐标开始寻找所有线索）
*@param 线索数量
*@return 从起始坐标到所有线索的路径（起始点->第一条线索->第二条线索->.....）每一段分别保存在vector数组中
*/
class clue_finder {
public:
    int clue_left_total; // 迷宫中线索的总数
    int size;             // 迷宫尺寸
    int (*maze)[MAXSIZE]; // 迷宫数组指针
    std::pair<int, int> locker;     // 初始机关坐标

private:
    int isvisited[MAXSIZE][MAXSIZE]; // 访问标记数组

    // 内部函数：初始化访问标记
    void init_visited() {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                isvisited[i][j] = 0;
            }
        }
    }

    // 核心回溯函数（设为私有，作为内部实现细节）
    bool find_clue_recursive(int x, int y, std::vector<std::pair<int, int>>& path) {
        if (x < 0 || x >= size || y < 0 || y >= size || isvisited[x][y] || maze[x][y] == static_cast<int>(MAZE::WALL)) {
            return false;
        }

        isvisited[x][y] = 1;
        path.push_back({x, y});

        if (maze[x][y] == static_cast<int>(MAZE::CLUE)) {
            return true;
        }

        if (find_clue_recursive(x + 1, y, path)) return true;
        if (find_clue_recursive(x - 1, y, path)) return true;
        if (find_clue_recursive(x, y + 1, path)) return true;
        if (find_clue_recursive(x, y - 1, path)) return true;

        path.pop_back();
        return false;
    }

public:
    // 构造函数
    clue_finder(int size, int (*maze)[MAXSIZE], std::pair<int, int> locker, int total_clues) {
        this->size = size;
        this->maze = maze;
        this->locker = locker;
        this->clue_left_total = total_clues;
    }

    // --- 新的公共接口函数 ---
    // 调用此函数来寻找所有线索的完整路径
    // 返回值: 一个向量，其中每个元素是另一条从一个点到下一个线索的路径
    std::vector<std::vector<std::pair<int, int>>> find_all_clue_paths() {
        std::vector<std::vector<std::pair<int, int>>> all_paths;
        std::pair<int, int> current_start_pos = this->locker;
        //std::cout<<"player current position: ("<<current_start_pos.first<<","<<current_start_pos.second<<")"<<std::endl;
        int clues_to_find = this->clue_left_total;

        while (clues_to_find > 0) {
            init_visited(); // 为下一次搜索重置访问数组
            std::vector<std::pair<int, int>> path_segment; // 存储当前路径段
            
            // 调用内部回溯函数寻找从当前起点到下一个线索的路径
            if (find_clue_recursive(current_start_pos.first, current_start_pos.second, path_segment)) {
                // 找到了一个线索
                all_paths.push_back(path_segment); // 将找到的路径段存入总路径容器

                // 更新下一个搜索的起点为刚刚找到的线索位置
                current_start_pos = path_segment.back();
                
                // 将已找到的线索在迷宫中标记为普通路径，以免重复查找
                // 注意：这会修改传入的迷宫数组
                maze[current_start_pos.first][current_start_pos.second] = static_cast<int>(MAZE::WAY);
                
                clues_to_find--;
            } else {
                // 如果找不到更多线索，说明剩余线索不可达，退出循环
                std::cout << "警告: 无法找到所有 " << clue_left_total << " 个线索，可能有部分被隔离。" << std::endl;
                break;
            }
        }
        
        // 恢复被修改的线索格，以便外部可以再次使用原始迷宫状态（可选）
        for(const auto& path_seg : all_paths) {
            if (!path_seg.empty()) {
                const auto& clue_pos = path_seg.back();
                maze[clue_pos.first][clue_pos.second] = static_cast<int>(MAZE::CLUE);
            }
        }

        return all_paths;
    }

    ~clue_finder() {}
};

int call_finder_example() {
    int size = 15;
    MazeGenerator generator(size);
    generator.generate();
    generator.placeFeatures();
    std::cout << "--- 初始迷宫 ---" << std::endl;
    generator.print();

    int (*maze)[MAXSIZE] = generator.getmaze();
    int size_of_maze = generator.getsize();
    int total_clues = 3; // 假设有3个线索

    // 创建 finder 实例
    clue_finder finder(size_of_maze, maze, generator.getLocker(), total_clues);//在此处输入开始坐标，开始寻找三个线索

    // --- 只需调用一个函数即可获取所有路径 ---
    std::cout << "\n--- 开始寻找所有线索的路径 ---" << std::endl;
    auto all_clue_paths = finder.find_all_clue_paths();//返回的是从你输入的坐标，途径三个线索的路线

    // --- 打印结果 ---
    if (all_clue_paths.empty()) {
        std::cout << "未能找到任何从机关到线索的路径。" << std::endl;
    } else {
        std::cout << "成功找到 " << all_clue_paths.size() << " 条路径段！" << std::endl;
        for (size_t i = 0; i < all_clue_paths.size(); ++i) {
            std::cout << "\n--- 路径段 " << i + 1 << " ---" << std::endl;
            const auto& path = all_clue_paths[i];
            for (size_t j = 0; j < path.size(); ++j) {
                std::cout << "(" << path[j].first << ", " << path[j].second << ")";
                if (j < path.size() - 1) {
                    std::cout << " -> ";
                }
            }
            std::cout << " (找到线索!)" << std::endl;
        }
    }

    std::cout << "\n--- 查找结束 ---" << std::endl;

    return 0;
}

#endif