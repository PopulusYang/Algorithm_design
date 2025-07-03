#pragma once
#include "player.h"
#include <cmath>
#include <vector>
#include <thread>
#include <string>

class autocontroller
{
public:
    enum class state
    {
        stop,
        up,
        down,
        right,
        left
    };

    friend std::ostream &operator<<(std::ostream &os, const state &s);

    autocontroller(player *p, GameController *mazeinformation = nullptr) : currentstate(state::stop), p(p), mazeinformation(mazeinformation)
    {
    }
    ~autocontroller()
    {
        if (auto_call_stop != nullptr)
        {
            if (auto_call_stop->joinable())
            {
                auto_call_stop->join();
                delete auto_call_stop; // 释放内存
            }
        }
    }
    void control(state newstate);
    void stopautocontrol();
    void moveforoneblock();
    void thread_auto_run();
    void runalongthePath(std::vector<point> path);

    std::thread *auto_call_stop = nullptr;

    std::mutex mtx, mtx2; // 互斥锁

    GameController *mazeinformation;
    bool rundone = false; // 用于标记是否已经完成自动控制

private:
    state currentstate;
    bool autorun = true; // 可作为线程退出开关
    player *p;
    QPointF lastpos;
    bool threadrunning = false;
};

inline std::ostream &operator<<(std::ostream &os, const autocontroller::state &s)
{
    std::string out;
    switch (s)
    {
    case autocontroller::state::up:
        out = "上";
        break;
    case autocontroller::state::down:
        out = "下";
        break;
    case autocontroller::state::left:
        out = "左";
        break;
    case autocontroller::state::right:
        out = "右";
        break;
    case autocontroller::state::stop:
        out = "停";
    }
    os << out;
    return os;
}

inline void autocontroller::moveforoneblock()
{
    bool released = false;
    while (autorun && !released)
    {
        std::lock_guard<std::mutex> lock(mtx2);
        double dx = p->playerPos.x() - lastpos.x();
        double dy = p->playerPos.y() - lastpos.y();
        double distance = std::hypot(dx, dy); // √(dx² + dy²)
        if (distance >= 1.0)                  // 适配无惯性
            released = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 减少CPU占用
    }

    // 移动结束后，将玩家精确地对齐到目标网格点
    QPointF targetPos = lastpos;
    switch (currentstate)
    {
    case state::up:
        targetPos.setY(round(lastpos.y()) - 1.0);
        targetPos.setX(round(lastpos.x()));
        break;
    case state::down:
        targetPos.setY(round(lastpos.y()) + 1.0);
        targetPos.setX(round(lastpos.x()));
        break;
    case state::left:
        targetPos.setX(round(lastpos.x()) - 1.0);
        targetPos.setY(round(lastpos.y()));
        break;
    case state::right:
        targetPos.setX(round(lastpos.x()) + 1.0);
        targetPos.setY(round(lastpos.y()));
        break;
    default:
        break; // 其他情况不做处理
    }
    p->playerPos = targetPos; // 强制校准位置

    currentstate = state::stop;
    threadrunning = false;
}

inline void autocontroller::control(autocontroller::state newstate)
{
    int destpos_x = 0, destpos_y = 0;
    switch (newstate)
    {
    case state::up:
        destpos_x = static_cast<int>(std::round(p->playerPos.x()));
        destpos_y = static_cast<int>(std::round(p->playerPos.y())) - 1;
        break;
    case state::down:
        destpos_x = static_cast<int>(std::round(p->playerPos.x()));
        destpos_y = static_cast<int>(std::round(p->playerPos.y())) + 1;
        break;
    case state::left:
        destpos_x = static_cast<int>(std::round(p->playerPos.x())) - 1;
        destpos_y = static_cast<int>(std::round(p->playerPos.y()));
        break;
    case state::right:
        destpos_x = static_cast<int>(std::round(p->playerPos.x())) + 1;
        destpos_y = static_cast<int>(std::round(p->playerPos.y()));
        break;
    default:
        return;
    }
    if (mazeinformation->maze[destpos_y][destpos_x] == static_cast<int>(MAZE::WALL)) // 不撞墙
        return;
    if (auto_call_stop != nullptr)
    {
        if (threadrunning)
        {
            return;
        }
        if (auto_call_stop->joinable())
        {
            auto_call_stop->join();
            delete auto_call_stop; // 释放内存
        }
    }
    lastpos = p->playerPos;
    currentstate = newstate;
    auto_call_stop = new std::thread([this]()
                                     { moveforoneblock(); }); // 新建线程
    threadrunning = true;
}
inline void autocontroller::stopautocontrol()
{
    autorun = false;
}

inline void autocontroller::thread_auto_run()
{
    state laststate = state::stop;
    int lastkey = 0;
    while (autorun)
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (currentstate != state::stop)
        {
            // 自动控制激活时，获取独占控制权
            p->pressedKeys.clear();
            switch (currentstate)
            {
            case state::up:
                p->pressedKeys.insert(Qt::Key_W);
                lastkey = Qt::Key_W;
                break;
            case state::down:
                p->pressedKeys.insert(Qt::Key_S);
                lastkey = Qt::Key_S;
                break;
            case state::left:
                p->pressedKeys.insert(Qt::Key_A);
                lastkey = Qt::Key_A;
                break;
            case state::right:
                p->pressedKeys.insert(Qt::Key_D);
                lastkey = Qt::Key_D;
                break;
            default:
                break;
            }
        }
        else
        {
            // 自动控制停止时，释放之前按下的键
            if (laststate != state::stop)
            {
                p->pressedKeys.remove(lastkey);
                lastkey = 0;
            }
            // 不执行任何操作，以允许手动键盘输入
        }
        laststate = currentstate;
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 等待50毫秒
    }
    // std::cout << "自动控制即将关闭" << std::endl;
}

inline void autocontroller::runalongthePath(std::vector<point> path)
{
  p->runalongthepath = true;
  rundone = false; // 重置标记
  // 先把vector拍成queue
  std::deque<point> temp_pathq
      = std::deque<point> (path.begin (), path.end ());
  std::queue<point> pathq (temp_pathq);
  state newstate;
  while (!pathq.empty ())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (threadrunning)
        {
            continue;
        }
        auto nextpos = pathq.front();
        pathq.pop();
        int player_pos_x = static_cast<int>(std::round(p->playerPos.x()));
        int player_pos_y = static_cast<int>(std::round(p->playerPos.y()));
        int dx = nextpos.x - player_pos_y;
        int dy = nextpos.y - player_pos_x;
        if (!dx && !dy)
            newstate = state::stop;
        else if (!dx)
        {
            if (dy > 0)
                newstate = state::right;
            else
                newstate = state::left;
        }
        else
        {
            if (dx > 0)
                newstate = state::down;
            else
                newstate = state::up;
        }

       // std::cout << "现在的行走方向:" << newstate << std::endl;
        control(newstate);
    }
  //  std::cout << "自动控制结束" << std::endl;
    p->runalongthepath = false;
    rundone = true; // 标记自动控制已完成
}
