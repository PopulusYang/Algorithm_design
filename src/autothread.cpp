#include "heads/autothread.h"
#include "heads/autocontrol.h"

AutoThread::AutoThread(autocontroller *ctrl, QObject *parent)
    : QThread(parent), ctrl(ctrl) {}

void AutoThread::run()
{
    if (ctrl)
        ctrl->thread_auto_run();
}
