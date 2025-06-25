#include "autowindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

AutoControlPanel::AutoControlPanel(autocontroller *controller, QWidget *parent)
    : QDialog(nullptr), controller(controller)
{
    setWindowTitle("自动控制面板");
    setModal(true); // 设置为模态对话框

    auto *layout = new QVBoxLayout(this);
    auto *btnUp = new QPushButton("上", this);
    auto *btnDown = new QPushButton("下", this);
    auto *btnLeft = new QPushButton("左", this);
    auto *btnRight = new QPushButton("右", this);
    auto *btnStop = new QPushButton("停", this);

    auto *hlayout = new QHBoxLayout();
    hlayout->addWidget(btnLeft);
    hlayout->addWidget(btnUp);
    hlayout->addWidget(btnDown);
    hlayout->addWidget(btnRight);
    layout->addLayout(hlayout);
    layout->addWidget(btnStop);

    connect(btnUp, &QPushButton::clicked, [this, controller]()
            { controller->control(autocontroller::state::up); });
    connect(btnDown, &QPushButton::clicked, [this, controller]()
            { controller->control(autocontroller::state::down); });
    connect(btnLeft, &QPushButton::clicked, [this, controller]()
            { controller->control(autocontroller::state::left); });
    connect(btnRight, &QPushButton::clicked, [this, controller]()
            { controller->control(autocontroller::state::right); });
    connect(btnStop, &QPushButton::clicked, [this, controller]()
            { controller->control(autocontroller::state::stop); });
}