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

void dp::isWorth(djstruct& input)
{
    if(*(input.path.end()-1) == end|| input.lenght==0)
        return;
    float cost = input.lenght;//路径代价
    float gain = sourse_value[*(input.path.end()-1)];//价值
    for(auto p: input.path)
    {
        if(traps.count(p))
            cost += 20.0;
    }
    float worth = gain / cost;
    if(worth<=0.5)
    {
        input.lenght = INF;
        std::cout << "该路径可收获" << gain << "，代价" << cost << ",worth:" << worth << std::endl;
    }
}

void dp::full_the_path(std::vector<point> &input)
{
    std::pair<point, point> road;
    std::vector<point> output = input;
    int size = (int)input.size();
    for (int i = size - 2; i >= 0; i--)//插入完整路径
    {
        road.first = *(input.begin() + i);
        road.second= *(input.begin() + i + 1);
        auto rpath = Dijkstra(road.first, road.second).path;
        output.insert(output.begin()+i+1,rpath.begin() + 1, rpath.end() - 1);
    }
    input = output;
}

djstruct dp::Dijkstra(point S, point E)
{
    djstruct res;
    int n = mazesize, m = mazesize;
    std::priority_queue<State> pq;
    std::vector<std::vector<int>> maxWeight(n, std::vector<int>(m, INT_MIN));

    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false));//已访问的点

    pq.push({S, weight(E, S), {S}});//将起点/初始状态放入队列
    maxWeight[S.x][S.y] = weight(E, S);//初始化dp表

    const std::vector<std::pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!pq.empty())//状态队列空即无解
    {
        auto [cur, curWeight, path] = pq.top();//获取当前状态
        pq.pop();//出队
        if (visited[cur.x][cur.y])
            continue;
        visited[cur.x][cur.y] = true;
        if (cur == E)//到达终点
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
                }
            }
        }
    }
    return {}; // 无解
}

std::vector<point> dp::findBestPath()
{
    int k = sourse.size();
    std::vector<point> R(sourse.begin(), sourse.end());//拍平成vector
    int V = k + 2;
    //预处理dist
    std::vector<std::vector<int>> dist(V, std::vector<int>(V, INF));

    auto node = [&](int idx) -> point
    {
        if(idx == 0)
            return start;
        else if(idx == V - 1)
            return end;
        else
            return R[idx - 1];
    };//lambda表达式，通过idx返回节点应该对应的地图坐标

    for (int i = 0; i < V; ++i)
        for (int j = i + 1; j < V; ++j)
        {
            int d = Dijkstra(node(i), node(j)).lenght; // 用Dijkstra获取两点之间的最短距离
            dist[i][j] = dist[j][i] = d;
        }

    int ALL = (1 << k) - 1;//ALL表示所有资源已被拾取

    std::vector<std::vector<int>> dp(1 << k, std::vector<int>(k, INF));//初始化dp表格

    std::vector<std::vector<std::pair<int, int>>> pre(1 << k, std::vector<std::pair<int, int>>(k, {-1, -1}));//回溯表
    for (int i = 0; i < k; ++i)
    {
        if (dist[0][i + 1] < INF)
            dp[1 << i][i] = dist[0][i + 1];
    }

    for (int mask = 1; mask <= ALL; ++mask)//枚举所有“已经收集的资源”的可能
    {
        for (int i = 0; i < k; ++i)
            if (mask & (1 << i) && dp[mask][i] < INF)//检查资源是否被收集，过滤无解
            {
                for (int j = 0; j < k; ++j)
                    if (!(mask & (1 << j)))//只考虑还没有被收集的资源
                    {
                        int nmask = mask | (1 << j);
                        if (dist[i + 1][j + 1] == INF)
                            continue; // i→j无法连通，跳过
                        int cand = dp[mask][i] + dist[i + 1][j + 1];
                        if (cand < dp[nmask][j])//更新状态
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
    
    //回溯得到访问顺序
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

    std::vector<point> fullPath = {start};
    for(int idx: orderR)
        fullPath.push_back(R[idx]);
    fullPath.push_back(end);
    std::cout << "补满前" << std::endl;
    for (auto p : fullPath)
    {
        std::cout << p.x << "," << p.y << std::endl;
    }

    full_the_path(fullPath);

    std::cout << "补满后" << std::endl;
    for (auto p : fullPath)
    {
        std::cout << p.x << "," << p.y << std::endl;
    }
    return fullPath;
}
