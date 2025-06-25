#ifndef PLAYER_H
#define PLAYER_H

#include <QPointF>
#include <QPixmap>
#include <QString>
#include <QSet>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <mutex>
#include <thread>
#include <chrono>

#include "gamemain.h"
#include "gamecontrol.h"

class player : public QObject
{
    Q_OBJECT
public:
    // 玩家相关
    QPointF playerPos;     // 玩家当前位置（浮点，便于惯性）
    QPointF playerVel;     // 玩家速度
    QPointF playerAcc;     // 玩家加速度
    QTimer *playerTimer;   // 控制玩家移动的定时器
    QSet<int> pressedKeys; // 当前按下的键
    float inertia;         // 惯性系数
    float moveSpeed;       // 基础速度

    QPixmap playerSprite;
    int playerDir = 2;            // 0:左 1:下 2:上 3:右
    int playerAnim = 0;           // 动画帧索引
    QString playerState = "idle"; // idle, walk, attack

    player(QObject *parent = nullptr) : QObject(parent) {}

    int animFrameCounter = 0;

public slots:
    void onPlayerMove(GameController *gameController);

signals:
    void needUpdate();
};


#endif