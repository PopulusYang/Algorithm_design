#include "player.h"

void player::onPlayerMove(GameController *gameController)
{
    // 计算加速度
    playerAcc = QPointF(0, 0);
    bool moving = false;
    if (pressedKeys.contains(Qt::Key_W))
    {
        playerAcc.ry() -= moveSpeed;
        playerDir = 2;
        moving = true;
    }
    if (pressedKeys.contains(Qt::Key_S))
    {
        playerAcc.ry() += moveSpeed;
        playerDir = 1;
        moving = true;
    }
    if (pressedKeys.contains(Qt::Key_A))
    {
        playerAcc.rx() -= moveSpeed;
        playerDir = 0;
        moving = true;
    }
    if (pressedKeys.contains(Qt::Key_D))
    {
        playerAcc.rx() += moveSpeed;
        playerDir = 3;
        moving = true;
    }

    // 更新状态
    if (moving)
    {
        playerState = "walk";
        animFrameCounter++;
        if (animFrameCounter >= 6)
        { // 每6帧才切换一次动画帧，数值越大动画越慢
            playerAnim = (playerAnim + 1) % 4;
            animFrameCounter = 0;
        }
    }
    else
    {
        playerState = "idle";
        playerAnim = 0;
        animFrameCounter = 0;
    }

    // 更新速度（惯性）
    playerVel = inertia * playerVel + (1 - inertia) * playerAcc;

    // 限制最大速度
    const float maxSpeed = 0.025f;
    if (std::hypot(playerVel.x(), playerVel.y()) > maxSpeed)
    {
        float scale = maxSpeed / std::hypot(playerVel.x(), playerVel.y());
        playerVel *= scale;
    }

    // 预测新位置
    QPointF nextPos = playerPos + playerVel;

    // 边界和障碍检测
    int nx = qRound(nextPos.y() + 0.15);
    int ny = qRound(nextPos.x() + 0.1);
    if (gameController && gameController->inBounds(nx, ny))
    {
        MAZE cell = static_cast<MAZE>(gameController->getMaze()[nx][ny]);
        if (cell != MAZE::WALL)
        {
            playerPos = nextPos;
        }
        else
        {
            playerVel = QPointF(0, 0); // 撞墙停下
        }
    }

    emit needUpdate();
}

