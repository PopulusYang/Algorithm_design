#include "dp.h"
constexpr int INF = 1e9;

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

void dp::isWorth(djstruct &input)
{
    if (*(input.path.end() - 1) == end || input.lenght == 0) // 如果路径终点是迷宫出口或者路径长度为0，跳过价值评估直接认可
        return;
    float cost = input.lenght;                          // 路径代价
    float gain = sourse_value[*(input.path.end() - 1)]; // 价值
    for (auto p : input.path)
    {
        if (traps.count(p)) // 踩到陷阱
            cost += 20.0;
    }
    float worth = gain / cost;
    if (worth <= 0.5)

    {
        input.lenght = INF;
        std::cout << "该路径可收获" << gain << "，代价" << cost << ",worth:" << worth << std::endl;
    }
}

void dp::full_the_path(std::vector<point> &input) // 将路连起来
{
    std::pair<point, point> road;
    std::vector<point> output = input;
    int size = (int)input.size();
    for (int i = size - 2; i >= 0; i--) // 插入完整路径
    {
        road.first = *(input.begin() + i);
        road.second = *(input.begin() + i + 1);
        auto rpath = Dijkstra(road.first, road.second).path;
        output.insert(output.begin() + i + 1, rpath.begin() + 1, rpath.end() - 1);
    }
    input = output;
}

// 为findBestPath新增一个只计算路径长度的Dijkstra版本
int dp::get_path_length(point S, point E)
{
    if (S == E)
        return 0;
    int n = mazesize, m = mazesize;
    std::queue<std::pair<point, int>> q;
    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false));

    q.push({S, 1}); // 路径长度从1开始
    visited[S.x][S.y] = true;

    const std::vector<std::pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!q.empty())
    {
        auto [cur, len] = q.front();
        q.pop();

        for (auto [dx, dy] : dirs)
        {
            int nx = cur.x + dx;
            int ny = cur.y + dy;

            if (!inBounds(nx, ny) || visited[nx][ny] || getCellWeight(static_cast<MAZE>(getMaze()[nx][ny])) <= -1000)
            {
                continue;
            }

            point next{nx, ny};
            if (next == E)
            {
                return len + 1;
            }

            visited[nx][ny] = true;
            q.push({next, len + 1});
        }
    }
    return INF; // 找不到路径
}

djstruct dp::Dijkstra(point S, point E) // 迪杰斯特拉算法求两点路径
{
    djstruct res;
    int n = mazesize, m = mazesize;
    std::priority_queue<State> pq;
    std::vector<std::vector<int>> maxWeight(n, std::vector<int>(m, INT_MIN));

    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false)); // 已访问的点

    pq.push({S, weight(E, S), {S}});    // 将起点/初始状态放入队列
    maxWeight[S.x][S.y] = weight(E, S); // 初始化dp表

    const std::vector<std::pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!pq.empty()) // 状态队列空即无解
    {
        auto [cur, curWeight, path] = pq.top(); // 获取当前状态
        pq.pop();                               // 出队
        if (visited[cur.x][cur.y])
            continue;
        visited[cur.x][cur.y] = true;
        if (cur == E) // 到达终点
        {
            res.lenght = (int)path.size();
            res.path = path;
            isWorth(res);
            return res;
        }
        if (path.size() > n * m * 2)
        {
            std::cerr << "路径太长，可能死循环，强制退出\n";
            return {};
        }

        for (auto [dx, dy] : dirs) // 扫描上下左右各个点
        {
            // 更新点坐标
            int nx = cur.x + dx;
            int ny = cur.y + dy;
            if (!inBounds(nx, ny)) // 越界处理
                continue;

            point next{nx, ny}; // 创建下一个点对象
            if (getCellWeight(static_cast<MAZE>(getMaze()[nx][ny])) <= -1000)
                continue; // 禁止通行

            int w = weight(end, next);  // 计算新的权值
            int totalW = curWeight + w; // 计算总权值

            if (totalW > maxWeight[nx][ny])
            {
                if (!visited[nx][ny] || totalW > maxWeight[nx][ny])
                {
                    maxWeight[nx][ny] = totalW;
                    auto newPath = path;
                    newPath.push_back(next);
                    pq.push({next, totalW, newPath}); // 创建新状态放入队列
                }
            }
        }
    }
    return {}; // 无解
}

//这个狗算法的时间复杂度竟然可以来到O( (k+2)^2 * BFS + 2^k * k^2 )，\
其中n和m分别是地图的行数和列数，完美解决动态规划算法优化的太好的问题
std::vector<point> dp::findBestPath(point playerstart, std::unordered_set<point> tempsource)
{
    int k = tempsource.size();
    std::vector<point> R(tempsource.begin(), tempsource.end()); // 拍平成vector
    int V = k + 2;
    // 预处理dist
    std::vector<std::vector<int>> dist(V, std::vector<int>(V, INF));

    auto node = [&](int idx) -> point
    {
        if (idx == 0)
            return playerstart;
        else if (idx == V - 1)
            return end;
        else
            return R[idx - 1];
    }; // lambda表达式，通过idx返回节点应该对应的地图坐标

    for (int i = 0; i < V; ++i)
        for (int j = i + 1; j < V; ++j)
        {
            // 使用新的BFS函数计算纯粹的路径长度
            int d = get_path_length(node(i), node(j));
            dist[i][j] = dist[j][i] = d;
        }

    int ALL = (1 << k) - 1; // ALL表示所有资源已被拾取

    std::vector<std::vector<int>> dp(1 << k, std::vector<int>(k, INF)); // 初始化dp表格

    std::vector<std::vector<std::pair<int, int>>> pre(1 << k, std::vector<std::pair<int, int>>(k, {-1, -1})); // 回溯表
    for (int i = 0; i < k; ++i)
    {
        if (dist[0][i + 1] < INF)
            dp[1 << i][i] = dist[0][i + 1];
    }

    for (int mask = 1; mask <= ALL; ++mask) // 枚举所有“已经收集的资源”的可能
    {
        for (int i = 0; i < k; ++i)
            if (mask & (1 << i) && dp[mask][i] < INF) // 检查资源是否被收集，过滤无解
            {
                for (int j = 0; j < k; ++j)
                    if (!(mask & (1 << j))) // 只考虑还没有被收集的资源
                    {
                        int nmask = mask | (1 << j);
                        if (dist[i + 1][j + 1] == INF)
                            continue; // i→j无法连通，跳过
                        int cand = dp[mask][i] + dist[i + 1][j + 1];
                        if (cand < dp[nmask][j]) // 更新状态
                        {
                            dp[nmask][j] = cand;
                            pre[nmask][j] = {mask, i};
                        }
                    }
            }
    }

    int bestEnd = -1, bestCost = INF;
    for (int i = 0; i < k; ++i)
    {
        int cand = dp[ALL][i] + dist[i + 1][V - 1];
        if (cand < bestCost)
        {
            bestCost = cand;
            bestEnd = i;
        }
    }

    // 回溯得到访问顺序
    std::vector<int> orderR;
    int curMask = ALL, cur = bestEnd;
    while (curMask != -1 && cur != -1)
    {
        orderR.push_back(cur);
        auto [pmask, prev] = pre[curMask][cur];
        curMask = pmask;
        cur = prev;
    }
    std::reverse(orderR.begin(), orderR.end());

    std::vector<point> fullPath = {};
    for (int idx : orderR)
        fullPath.push_back(R[idx]);
    fullPath.push_back(end);
    std::cout << "补满前" << std::endl;
    for (auto p : fullPath)
    {
        std::cout << p.x << "," << p.y << std::endl;
    }

    // full_the_path(fullPath);

    // std::cout << "补满后" << std::endl;
    // for (auto p : fullPath)
    // {
    //     std::cout << p.x << "," << p.y << std::endl;
    // }
    return fullPath;
}

std::vector<point> dp::simulate(point playerstart)
{
    collecter.tempset = this->sourse;
    std::vector<point> path;
    std::vector<point> finalpath;
    point current = playerstart;
    path = findBestPath(current, collecter.tempset);
    while (current != end)
    {
        
        point startp = current;
        point endp = path[0];

        path.erase(path.begin()); // 移除第一个点
        auto [length, p] = Dijkstra(startp, endp); // 获取路径
        
        // 用贪心算法收集金币
        std::cout << "开始贪婪" << std::endl;
        current = endp;
        
        std::vector<point> has_collected; // 记录已收集的资源点

        for (int i = 0; i < p.size(); i++)
        {
            if (collecter.tempset.count(p[i]))
            {
                collecter.tempset.erase(p[i]); // 记录已收集资源
                has_collected.push_back(p[i]);
            }
        }

        

        std::cout << "当前计算路径" << std::endl;
        for (auto p : p)
        {
            std::cout << p.x << "," << p.y << std::endl;
        }

        finalpath.insert(finalpath.end(), p.begin() + 1, p.end());


        if (current == end)
            break;
        int source_count = 0;
        int loop_count = 0;

        while (collecter.ifsourvaild(current)) // 如果当前点周围存在资源
        {
            current = collecter.findway(current); // 找到下一个资源点
            point next = collecter.findway(current); // 找到下一个资源点
            
            if (collecter.tempset.count(current))
            {
                collecter.tempset.erase(current); // 记录已收集资源
                has_collected.push_back(current);
                std::cout << "贪婪收集到资源点" << current.x << "," << current.y << std::endl;
                source_count++;
            }
            if (loop_count>14||next == current)
            { // 如果位置没有变化，说明卡住了，退出贪心
                std::cout << "贪婪算法卡住，退出循环" << std::endl;
                break;
            }
            finalpath.push_back(current); // 将资源点加入路径
            loop_count++;
        }
        std::cout << "结束贪婪，通过贪心算法收集到金币：" << source_count << std::endl;
        std::vector<point> temp = path;
        for (auto it : has_collected) // 遍历已收集的资源点
        {
            int index = 0;
            for(auto po : temp)
            {
                if (it == po)
                {
                    path.erase(path.begin() + index); // 移除已收集的资源点
                    index--;
                }
                index++;
            }
        }
    }
    std::cout << "输出路径" << std::endl;
    for (auto p : finalpath)
    {
        std::cout << p.x << "," << p.y << std::endl;
    }
    return finalpath;
}
