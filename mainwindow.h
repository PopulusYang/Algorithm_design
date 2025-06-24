#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapbuild.h" // 包含地图生成器
#include "dp.h"
#include <QPainter>    // 包含 QPainter
#include <QPaintEvent> // 包含 QPaintEvent
#include <QPushButton>
#include <vector>
#include <QTimer>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QPixmap>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class GameController : public MazeGenerator, public dp
{
public:
    GameController(int size) : gamemain(size), MazeGenerator(size), dp(size) {}
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

private slots:
    void onSolveMazeClicked();
    void onPlayerMove();

private:
    Ui::MainWindow *ui;
    GameController *gameController;
    int blockSize;
    QPushButton *solveButton;
    std::vector<point> solvedPath;
    int animFrameCounter = 0;
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
};
#endif // MAINWINDOW_H
