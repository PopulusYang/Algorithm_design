#pragma once
#include "player.h"
#include <cmath>
#include <vector>
#include <thread>

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
    autocontroller(player *p, GameController *mazeinformation = nullptr) : currentstate(state::stop), p(p), mazeinformation(mazeinformation) {}
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

    std::thread *auto_call_stop = nullptr;

    std::mutex mtx, mtx2; // 互斥锁

    GameController *mazeinformation;

private:
    state currentstate;
    bool autorun = true; // 可作为线程退出开关
    player *p;
    QPointF lastpos;
    bool threadrunning = false;
};

inline void autocontroller::moveforoneblock()
{
    bool released = false;
    while (autorun && !released)
    {
        std::lock_guard<std::mutex> lock(mtx2);
        double dx = p->playerPos.x() - lastpos.x();
        double dy = p->playerPos.y() - lastpos.y();
        double distance = std::hypot(dx, dy); // √(dx² + dy²)
        if (distance >= 0.825)
            released = true;
    }
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
    if (mazeinformation->getMaze()[destpos_y][destpos_x] == static_cast<int>(MAZE::WALL)) // 不撞墙
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
    std::cout << "自动控制即将关闭" << std::endl;
}