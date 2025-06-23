#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <algorithm>
#include <utility>
#include "mapbuild.h"
// 为了符合题目要求，定义一个固定的MAXSIZE，但内部使用动态的std::vector<std::string>
// 这样做更灵活且是C++的最佳实践。


int call_mapbuild_example() {
    int size;
    std::cout << "请输入迷宫的尺寸 (推荐奇数, 最小为7): ";
    std::cin >> size;

    std::cout << "\n正在生成 " << size << "x" << size << " (或调整后的尺寸) 的迷宫...\n" << std::endl;

    // 创建生成器实例
    MazeGenerator generator(size);

    // 生成迷宫结构
    generator.generate();
    
    // 随机放置各种物件
    generator.placeFeatures();

    //获取迷宫数组,打印maze[0][0]
    std::cout<<generator.getMaze()[0][0];

    // 打印最终的迷宫
    std::cout << "迷宫生成完毕:" << std::endl;
    std::cout << "S: 起点, E: 终点, #: 墙壁, G: 金币, T: 陷阱, L: 机关, B: BOSS" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    generator.print();
    std::cout << "-------------------------------------------------" << std::endl;
    generator.print_num();

    return 0;
}