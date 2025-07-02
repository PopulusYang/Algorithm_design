#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>
#include <QPixmap>
#include <QMutex>
#include <QWaitCondition>
#include <QPainter>
#include "gamecontrol.h"
#include "player.h"
#include <list>

// 用于在线程间传递渲染所需的所有数据
class SceneData
{
public:
    // 游戏状态
    GameController *gameController = nullptr; // 游戏控制器
    point start;
    point end;
    std::vector<point> solvedPath;
    std::vector<std::vector<std::pair<int, int>>> cluePath;

    std::list<DamageIndicator> *damageIndicators = nullptr;
    // 玩家状态
    player *playerData;

    // 视图状态
    int blockSize;
    bool isGenerating;
    QSize windowSize;
    float camX;
    float camY;

    // 动画状态
    int bossAnim;
    int goldAnim;
    int clueAnim;
    SceneData &operator=(const SceneData &rhs);
    SceneData(const SceneData &rhs)
        : start(rhs.start), end(rhs.end),
          solvedPath(rhs.solvedPath), cluePath(rhs.cluePath),
          playerData(rhs.playerData), blockSize(rhs.blockSize),
          isGenerating(rhs.isGenerating), windowSize(rhs.windowSize),
          camX(rhs.camX), camY(rhs.camY), bossAnim(rhs.bossAnim), goldAnim(rhs.goldAnim), gameController(rhs.gameController), damageIndicators(rhs.damageIndicators),clueAnim(rhs.clueAnim) {}
    SceneData()
        : start(0, 0), end(0, 0), blockSize(32),
          isGenerating(false), windowSize(800, 600), camX(0), camY(0), bossAnim(0), goldAnim(0){}
};

inline SceneData &SceneData::operator=(const SceneData &rhs)
{
    if (this != &rhs)
    {
        gameController = rhs.gameController;
        start = rhs.start;
        end = rhs.end;
        solvedPath = rhs.solvedPath;
        cluePath = rhs.cluePath;
        playerData = rhs.playerData;
        blockSize = rhs.blockSize;
        isGenerating = rhs.isGenerating;
        windowSize = rhs.windowSize;
        camX = rhs.camX;
        camY = rhs.camY;
        bossAnim = rhs.bossAnim;
        goldAnim = rhs.goldAnim;
        clueAnim = rhs.clueAnim;
        damageIndicators = rhs.damageIndicators;
    }
    return *this;
}

class RenderThread : public QThread
{
    Q_OBJECT
public:
    RenderThread(QObject *parent = nullptr);
    ~RenderThread();

    void run() override;
    void stop();
    void requestFrame(const SceneData &data);

signals:
    void frameReady(const QPixmap &frame);

private:
    void renderScene(QPainter *painter, const SceneData &data);

    QMutex mutex;
    QWaitCondition cond;
    bool m_running = true;
    bool m_frameRequested = false;

    SceneData m_sceneData;

    // 预加载资源
    QPixmap m_playerSprite;
    QPixmap m_monsterSheet;
    QPixmap m_wallPixmap;
    QPixmap m_startPixmap;
    QPixmap m_exitPixmap;
    QPixmap m_goldPixmap;
    QPixmap m_lockerPixmap;
    QPixmap m_cluePixmap;
    QPixmap m_trapPixmap;
};

#endif // RENDERTHREAD_H