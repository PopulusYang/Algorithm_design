#include "renderthread.h"

RenderThread::RenderThread(QObject *parent)
    : QThread(parent)
{
    // 在构造函数中预加载所有图像资源
    m_playerSprite.load("./img/player.png");
    m_monsterSheet.load("./img/monster.png");
    m_wallPixmap.load("./img/wall.png");
    m_startPixmap.load("./img/start.png");
    m_exitPixmap.load("./img/exit.png");
    m_goldPixmap.load("./img/gold.png");
    m_lockerPixmap.load("./img/locker.png");
    m_cluePixmap.load("./img/clue.png");
    m_trapPixmap.load("./img/trap.png");
}

RenderThread::~RenderThread()
{
    stop();
    wait();
}

void RenderThread::stop()
{
    QMutexLocker locker(&mutex);
    m_running = false;
    cond.wakeOne();
}

void RenderThread::requestFrame(const SceneData &data)
{
    QMutexLocker locker(&mutex);
    m_sceneData = data;
    m_frameRequested = true;
    cond.wakeOne();
}

void RenderThread::run()
{
    while (m_running)
    {
        mutex.lock();
        if (!m_frameRequested)
        {
            cond.wait(&mutex);
        }
        if (!m_running)
        {
            mutex.unlock();
            break;
        }

        SceneData currentData = m_sceneData;
        m_frameRequested = false;
        mutex.unlock();

        QPixmap frame(currentData.windowSize);
        frame.fill(Qt::transparent);

        QPainter painter(&frame);
        renderScene(&painter, currentData);

        emit frameReady(frame);
    }
}

void RenderThread::renderScene(QPainter *painter, const SceneData &data)
{
    // 此函数包含从 MainWindow::paintEvent 移动过来的所有绘图逻辑
    painter->setRenderHint(QPainter::Antialiasing);

    // --- 1. 准备玩家渲染数据 ---
    // 提前计算好玩家要显示的精灵图部分，因为后面会用到
    const int playerSpriteW = 600 / 10;
    const int playerSpriteH = 290 / 4;
    int dir = data.playerData->playerDir;
    int col = 0;
    if (data.playerData->playerState == "idle")
        col = 0 + (data.playerData->playerAnim % 2);
    else if (data.playerData->playerState == "walk")
        col = 2 + (data.playerData->playerAnim % 4);
    QRect playerSrcRect(col * playerSpriteW, dir * playerSpriteH, playerSpriteW, playerSpriteH);

    // --- 2. 渲染世界（地图、物品等） ---
    painter->save(); // 保存当前状态，以便后续恢复

    if (!data.isGenerating)
    {
        // 计算一个动态的缩放因子，以抵消blockSize变化带来的影响
        // 目标是让渲染出的元素尺寸与 blockSize 为 63 (mazesize=11时的基准值) 时保持一致
        const float baseBlockSize = 63.0f;
        float dynamicScale = baseBlockSize / static_cast<float>(data.blockSize);

        // 【修复】将相机坐标取整，防止抖动
        qreal camX = round(data.camX);
        qreal camY = round(data.camY);

        // 应用相机缩放和位移
        painter->scale(dynamicScale, dynamicScale);
        painter->translate(-camX, -camY);
    }

    // 在应用了变换之后，计算玩家的世界坐标中心点应该被映射到屏幕的哪个位置
    QPointF playerWorldCenter(
        data.playerData->playerPos.x() * data.blockSize + data.blockSize / 2.0,
        data.playerData->playerPos.y() * data.blockSize + data.blockSize / 2.0);
    QPointF playerScreenCenter = painter->worldTransform().map(playerWorldCenter);
    // 【修复】将玩家屏幕坐标取整，解决玩家抖动问题
    playerScreenCenter.setX(round(playerScreenCenter.x()));
    playerScreenCenter.setY(round(playerScreenCenter.y()));

    // 循环绘制地图的各个地块
    const int subBlockSize = data.blockSize / 3;
    for (int i = 0; i < data.gameController->mazesize; ++i)
    {
        for (int j = 0; j < data.gameController->mazesize; ++j)
        {
            QRect blockRect(j * data.blockSize, i * data.blockSize, data.blockSize, data.blockSize);
            MAZE blockType = static_cast<MAZE>(data.gameController->maze[i][j]);

            // 先绘制一个深灰色底板作为通路
            painter->fillRect(blockRect, Qt::darkGray);

            if (blockType == MAZE::WALL)
            {
                painter->drawPixmap(blockRect, m_wallPixmap);
            }
            else if (blockType == MAZE::START)
            {
                painter->drawPixmap(blockRect, m_startPixmap);
            }
            else if (blockType == MAZE::EXIT)
            {
                painter->drawPixmap(blockRect, m_exitPixmap);
            }
            else if (blockType == MAZE::SOURCE)
            {
                int frameW = m_goldPixmap.width() / 18; // 精灵图有18帧
                int frameH = m_goldPixmap.height();
                int frameIndex = data.goldAnim % 18; // 使用传入的动画帧索引
                QRect srcRect(frameIndex * frameW, 0, frameW, frameH);

                // 创建一个更小的矩形来绘制金币，使其不填满整个格子
                int goldSize = data.blockSize / 3; // 将金币大小设置为格子的2/3
                int offset = (data.blockSize - goldSize) / 2;
                QRect goldRect(blockRect.x() + offset, blockRect.y() + offset, goldSize, goldSize);

                painter->drawPixmap(goldRect, m_goldPixmap, srcRect);
            }
            else if (blockType == MAZE::LOCKER)
            {
                painter->drawPixmap(blockRect, m_lockerPixmap);
            }
            else if (blockType == MAZE::CLUE)
            {
                int frameW = m_cluePixmap.width() / 8; // 精灵图有8帧
                int frameH = m_cluePixmap.height();
                int frameIndex = data.clueAnim; // 使用传入的动画帧索引
                QRect srcRect(frameIndex * frameW, 0, frameW, frameH);
                // 创建一个更小的矩形来绘制金币，使其不填满整个格子
                int goldSize = data.blockSize / 3; // 将金币大小设置为格子的2/3
                int offset = (data.blockSize - goldSize) / 2;
                QRect goldRect(blockRect.x() + offset, blockRect.y() + offset, goldSize, goldSize);
                painter->drawPixmap(goldRect, m_cluePixmap, srcRect);
            }
            else if (blockType == MAZE::TRAP)
            {
                // 只有当陷阱被触发后才渲染
                point trap_pos(i, j);
                if (data.gameController->traps.count(trap_pos))
                {
                    if (data.gameController->traps[trap_pos])
                        painter->drawPixmap(blockRect, m_trapPixmap);
                }
            }
            else if (blockType == MAZE::BOSS)
            {
                // 绘制Boss，并处理动画
                int frameW = m_monsterSheet.width() / 8; // 精灵图有8帧
                int frameH = m_monsterSheet.height();
                int frameIndex = data.bossAnim % 8; // 使用传入的动画帧索引
                QRect srcRect(frameIndex * frameW, 0, frameW, frameH);
                painter->drawPixmap(blockRect, m_monsterSheet, srcRect);
            }
        }
    }

    // 绘制路径...
    if (!data.solvedPath.empty())
    {
        // ...
    }

    painter->restore(); // 恢复到save()之前的状态，即移除了缩放和位移

    // --- 3. 渲染玩家 ---
    // 现在我们在屏幕坐标系下绘制，玩家大小将是固定的像素值
    const int playerRenderSize = 48; // 玩家在屏幕上的固定渲染大小（例如48x48像素）
    QRectF playerDestRect(0, 0, playerRenderSize, playerRenderSize);
    playerDestRect.moveCenter(playerScreenCenter); // 将玩家矩形的中心移动到之前计算好的屏幕位置
    painter->drawPixmap(playerDestRect, m_playerSprite, playerSrcRect);

    // 绘制伤害数字
    if (data.damageIndicators)
    {
        painter->setPen(Qt::red);
        painter->setFont(QFont("Arial", 16, QFont::Bold));
        for (const auto &indicator : *data.damageIndicators)
        {
            // 将伤害数字的世界坐标转换为屏幕坐标
            QPointF textWorldPos(
                indicator.position.x() * data.blockSize + data.blockSize,
                indicator.position.y() * data.blockSize);
            QPointF textScreenPos = painter->worldTransform().map(textWorldPos);

            // 根据生命周期计算透明度和位置偏移
            float progress = static_cast<float>(indicator.lifetime) / 60.0f;
            QColor color = painter->pen().color();
            color.setAlphaF(progress); // 淡出效果
            painter->setPen(color);

            textScreenPos.ry() -= (1.0f - progress) * 20.0f; // 向上漂浮

            painter->drawText(textScreenPos, indicator.text);
        }
    }

    // --- 4. 渲染UI覆盖层（如渐变遮罩） ---
    if (!data.isGenerating)
    {
        QRadialGradient grad(data.windowSize.width() / 2, data.windowSize.height() / 2, data.windowSize.width() / 2.2);
        grad.setColorAt(0.7, QColor(0, 0, 0, 0));    // 中心区域更亮
        grad.setColorAt(0.95, QColor(0, 0, 0, 150)); // 边缘渐变
        grad.setColorAt(1, QColor(0, 0, 0, 220));    // 最外层更暗
        painter->setBrush(grad);
        painter->drawRect(QRect(QPoint(0, 0), data.windowSize));
    }
    // 显示当前资源数
    painter->setPen(Qt::yellow);
    painter->setFont(QFont("Arial", 14, QFont::Bold));
    QString resourceText = QString("金钱: %1").arg(data.playerData->playersource);
    painter->drawText(20, 600, resourceText);
}