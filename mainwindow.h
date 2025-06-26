#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>    // 包含 QPainter
#include <QPaintEvent> // 包含 QPaintEvent
#include <QPushButton>
#include <vector>
#include <QTimer>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QPixmap>
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include "player.h"
#include "gamecontrol.h"
#include "autowindow.h"
#include "autocontrol.h"
#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MonsterRenderThread : public QThread
{
    Q_OBJECT
public:
    MonsterRenderThread(QObject *parent = nullptr);
    void requestFrame(int anim, const QRect &srcRect, const QSize &targetSize);
    QPixmap getResult();
    void run() override;
    void stop();

signals:
    void frameReady(); // 添加这一行

private:
    QMutex mutex;
    QWaitCondition cond;
    bool running = true;
    int anim = 0;
    QRect srcRect;
    QSize targetSize;
    QPixmap result;
    bool frameRequested = false;
    QPixmap monsterSheet;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override; // 用于关闭输入法
    void createAutoControlPanel();

private slots:
    void onSolveMazeClicked();

    void ontimeout();

private:
    Ui::MainWindow *ui;
    GameController *gameController;
    int blockSize;
    QPushButton *solveButton;
    std::vector<point> solvedPath;

    player Player;

    // Boss 相关
    QPointF bossPos;
    QPixmap bossSprite;
    int bossDir = 2;
    int bossAnim = 0;
    int bossAnimFrameCounter = 0;

    MonsterRenderThread *monsterThread;
    QPixmap monsterFrameReady;
    QMutex monsterFrameMutex;

    autocontroller autoCtrl = autocontroller(&Player);
    std::thread *autoThread = nullptr;
    std::thread *panelThread = nullptr;
    AutoControlPanel *autoPanel = nullptr;

signals:
    void needMove(GameController *gameController);
};
#endif // MAINWINDOW_H
