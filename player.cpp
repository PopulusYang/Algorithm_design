#include "player.h"
#include <windows.h>
#include <QMessageBox>
#include <QInputDialog>

//在玩家接触到线索时，将此时的线索信息储存起来
void record_clue(GameController *gameController,double xd,double yd){
    int x=(int)xd;
    int y=(int)yd;
    point temp_point;
    temp_point.x=x;
    temp_point.y=y;
    auto it = gameController->clue_set.find(temp_point);
    //std::cout<<"match hash list in "<<it->second.gen_order_index<<std::endl;
    if(it == gameController->clue_set.end()) return; // 没有这个线索点
    const auto& clue = it->second;
    // 检查是否已加入过
    bool already = false;
    for(const auto& c : gameController->received_clue){
        if(c.gen_order_index == clue.gen_order_index && c.password_dig_val == clue.password_dig_val) {
            already = true;
            break;
        }
    }
    if(!already) {
        gameController->received_clue.push_back(clue);
        //std::cout<<"recorded a clue, now i know the "<<clue.gen_order_index<<" of password is:"<<clue.password_dig_val<<std::endl;
    }
}

void locker_interaction(GameController *gameController){
    gameController->is_near_locker=true;
}

void locker_leave(GameController *gameController){
    gameController->is_near_locker=false;
}

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
        if (animFrameCounter >= 2)
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
    const float maxSpeed = 0.05f;
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

    int cur_x = qRound(playerPos.y() + 0.15);
    int cur_y = qRound(playerPos.x() + 0.1);
    if (static_cast<MAZE>(gameController->getMaze()[cur_x][cur_y]) == MAZE::CLUE) {
        //std::cout<<"find a clue in :("<<cur_x<<","<<cur_y<<")"<<std::endl;
        record_clue(gameController, cur_x, cur_y);
    }

    if(static_cast<MAZE>(gameController->getMaze()[cur_x][cur_y]) == MAZE::LOCKER){
        //std::cout<<"find locker in :("<<cur_x<<","<<cur_y<<")"<<std::endl;
        locker_interaction(gameController);
    }
    else{
        locker_leave(gameController);
    }

    if (gameController && gameController->inBounds(nx, ny))
    {
        MAZE cell = static_cast<MAZE>(gameController->getMaze()[nx][ny]);
        // 检查当前位置是否为线索点

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



