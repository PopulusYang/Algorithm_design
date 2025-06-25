#pragma once
#include "player.h"

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
    autocontroller() : currentstate(state::stop) {}
    void control(state newstate);
    void stopautocontrol();

    void thread_auto_run(player &p);

    std::mutex mtx; // 互斥锁

private:
    state currentstate;
    bool autorun = true; // 可作为线程退出开关
};

inline void autocontroller::control(autocontroller::state newstate)
{
    currentstate = newstate;
}
inline void autocontroller::stopautocontrol()
{
    autorun = false;
}

inline void autocontroller::thread_auto_run(player &p)
{
    state laststate = currentstate;
    int lastkey = 0;
    while (autorun)
    {
        std::lock_guard<std::mutex> lock(mtx);
        switch (currentstate) // 模拟键盘按下
        {
        case state::stop:
            p.pressedKeys.clear();
            break;
        case state::up:
            p.pressedKeys.clear();
            if (!p.pressedKeys.contains(Qt::Key_W))
                p.pressedKeys.insert(Qt::Key_W);
            lastkey = Qt::Key_W;
            break;
        case state::down:
            p.pressedKeys.clear();
            if (!p.pressedKeys.contains(Qt::Key_S))
                p.pressedKeys.insert(Qt::Key_S);
            lastkey = Qt::Key_S;
            break;
        case state::left:
            p.pressedKeys.clear();
            if (!p.pressedKeys.contains(Qt::Key_A))
                p.pressedKeys.insert(Qt::Key_A);
            lastkey = Qt::Key_A;
            break;
        case state::right:
            p.pressedKeys.clear();
            if (!p.pressedKeys.contains(Qt::Key_D))
                p.pressedKeys.insert(Qt::Key_D);
            lastkey = Qt::Key_D;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 等待50毫秒
    }
    std::cout << "自动控制即将关闭" << std::endl;
}