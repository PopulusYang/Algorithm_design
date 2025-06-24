#include "mapbuild.h"
/*
@breif 
*/
class clue_finder {
public:
int clue_left=3; // 剩余线索数量
int size; // 迷宫尺寸
int (*maze)[MAXSIZE]; // 迷宫数组指针
int isvisited[MAXSIZE][MAXSIZE]; // 访问标记数组
std::pair<int, int> locker; // 机关坐标
std::pair<int, int> current_position; // 当前查找位置
clue_finder(int size, int (*maze)[MAXSIZE],std::pair<int,int> locker) {
    // 构造函数可以初始化一些必要的参数
    this->size = size;
    this->maze = maze;      
    this->locker = locker; // 机关坐标
}

void init_visited() {
    // 初始化访问标记数组
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            isvisited[i][j] = 0; // 未访问标记为0
        }
    }
}

void find_clue(int x, int y) {
    //std::cout << "当前位置: (" << x << ", " << y << ")" << std::endl;
    // 递归查找线索的函数
    if (x < 0 || x >= size || y < 0 || y >= size || isvisited[x][y] || maze[x][y] == static_cast<int>(MAZE::WALL)) {
        return ; // 越界或已访问或是墙壁
    }
    
    isvisited[x][y] = 1; // 标记为已访问

    // 检查当前格子是否是线索
    if (maze[x][y] == static_cast<int>(MAZE::CLUE)) {
        std::cout << "找到线索在位置: (" << x << ", " << y << ")" << std::endl;
        clue_left--; // 减少剩余线索数量
        if (clue_left <= 0) {
            std::cout << "所有线索已找到！" << std::endl;
            current_position = std::make_pair(x, y);
            return ; // 找到所有线索后返回
        }
        //return; // 找到线索后返回
    }

    // 递归探索四个方向
    find_clue(x + 1, y); // 下
    find_clue(x - 1, y); // 上
    find_clue(x, y + 1); // 右
    find_clue(x, y - 1); // 左
}

std::pair<int,int> getpos() {
    // 返回当前查找位置
    return current_position;
   
}

~clue_finder() {
    // 析构函数可以释放资源
}
};


int main() {
    int size = 15; // 迷宫尺寸
    MazeGenerator generator(size); // 创建生成器实例
    generator.generate();          // 生成迷宫结构
    generator.placeFeatures();     // 随机放置物件
    generator.print();             // 打印迷宫（可选）

    // 获取迷宫数组指针
    int (*maze)[MAXSIZE] = generator.getmaze();
    int size_of_maze = generator.getsize();
    // 例如访问某个格子的类型
    clue_finder finder(size_of_maze, maze, generator.getLocker());
    finder.init_visited(); // 初始化访问标记数组
    finder.find_clue(generator.getLocker().first, generator.getLocker().second); // 从机关位置(或者其他位置)开始查找线索，不管金币，陷阱这些玩意的收益，因为线索必须找到
    std::pair<int, int> pos = finder.getpos(); // 获取最后查找位置
    std::cout<< "查找线索结束，最后位置: (" << pos.first << ", " << pos.second << ")" << std::endl;
    return 0;
}