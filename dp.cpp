#include "dp.h"


int dp::getCellWeight(MAZE cellType) const
{
    switch (cellType)
    {
    case MAZE::SOURCE:
        return 500; // 资源高优先级
    case MAZE::WAY:
        return 0; // 普通路径
    case MAZE::TRAP:
        return -300; // 陷阱避开
    case MAZE::WALL:
        return -1000; // 禁止通行
    case MAZE::EXIT:
        return 2000; // 出口最高优先级
    default:
        return 0;
    }
}

int dp::weight(point dest, point current) const
{
    double dx = dest.x - current.x;
    double dy = dest.y - current.y;
    int distance_weight = 1000 - (int)(sqrt(dx * dx + dy * dy) * 10);

    // 格子类型权值
    MAZE cellType = static_cast<MAZE>(getMaze()[current.x][current.y]);
    int cell_weight = getCellWeight(cellType);

    // 附加资源点权值
    double resource_weight = 0.0;
    const double RESOURCE_BASE = 300.0; // 每个资源点的基础影响力
    const double DECAY_RATE = 20.0;     // 衰减因子，越大衰减越快
    const double EPSILON = 1e-6;        // 防止除零

    for (const point &src : sourse) // source 为资源点集合
    {
        double ddx = current.x - src.x;
        double ddy = current.y - src.y;
        double dist = sqrt(ddx * ddx + ddy * ddy);

        resource_weight += RESOURCE_BASE / (dist + EPSILON); // 可根据需要换成其他形式
    }

    return distance_weight + cell_weight + (int)resource_weight;
}

std::vector<point> dp::findBestPath()
{
    int n = mazesize, m = mazesize;
    std::priority_queue<State> pq;
    std::vector<std::vector<int>> maxWeight(n, std::vector<int>(m, INT_MIN));

    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false));//已访问的点

    pq.push({start, weight(end, start), {start}});//将起点/初始状态放入队列
    maxWeight[start.x][start.y] = weight(end, start);//初始化dp表

    const std::vector<std::pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!pq.empty())//状态队列空即无解
    {
        auto [cur, curWeight, path] = pq.top();//获取当前状态
        pq.pop();//出队
        if (visited[cur.x][cur.y])
            continue;
        visited[cur.x][cur.y] = true;
        if (cur == end)//到达终点
        {
            printDPTable(maxWeight);
            return path;
        }
        if (path.size() > n * m * 2)
        {
            std::cerr << "路径太长，可能死循环，强制退出\n";
            return {};
        }
        
        for (auto [dx, dy] : dirs)//扫描上下左右各个点
        {
            //更新点坐标
            int nx = cur.x + dx;
            int ny = cur.y + dy;
            if (!inBounds(nx, ny))//越界处理
                continue;

            point next{nx, ny};//创建下一个点对象
            if (getCellWeight(static_cast<MAZE>(getMaze()[nx][ny])) <= -1000)
                continue; // 禁止通行

            int w = weight(end, next);//计算新的权值
            int totalW = curWeight + w;//计算总权值

            if (totalW > maxWeight[nx][ny])
            {
                if (!visited[nx][ny] || totalW > maxWeight[nx][ny])
                {
                    maxWeight[nx][ny] = totalW;
                    auto newPath = path;
                    newPath.push_back(next);
                    pq.push({next, totalW, newPath}); // 创建新状态放入队列
                    std::cout << "最新状态坐标：" << nx << ',' << ny << "\t最新权值:" << totalW << std::endl;
                }
            }
        }
    }
    printDPTable(maxWeight);
    return {}; // 无解
}

void dp::printDPTable(const std::vector<std::vector<int>> &maxWeight) const
{
    for (const auto &row : maxWeight)
    {
        for (int val : row)
        {
            if (val == INT_MIN)
                std::cout << "####\t"; // 表示未访问
            else
                std::cout << val << "\t";
        }
        std::cout << "\n";
    }
}
