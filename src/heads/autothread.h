#ifndef AUTOTHREAD_H
#define AUTOTHREAD_H

#include <QThread>
#include "autocontrol.h"
#include "gamemain.h"


class AutoThread : public QThread
{
    Q_OBJECT
public:
    AutoThread(autocontroller *ctrl, QObject *parent = nullptr);
    void run() override;

private:
    autocontroller *ctrl;
};

// --- 新增：RunalongThread 封装自动寻路线程 ---
class RunalongThread : public QThread
{
    Q_OBJECT
public:
    RunalongThread(autocontroller *ctrl, const std::vector<point> &path, QObject *parent = nullptr)
        : QThread(parent), ctrl(ctrl), path(path) {}
    void run() override
    {
        if (ctrl)
            ctrl->runalongthePath(path);
    }

private:
    autocontroller *ctrl;
    std::vector<point> path;
};

#endif // AUTOTHREAD_H
