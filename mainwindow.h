#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapbuild.h" // 包含地图生成器
#include "dp.h"
#include <QPainter>    // 包含 QPainter
#include <QPaintEvent> // 包含 QPaintEvent
#include <QPushButton>
#include <vector>

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
    // 重写 paintEvent 来绘制迷宫
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onSolveMazeClicked();

private:
    Ui::MainWindow *ui;
    GameController *gameController; // 游戏控制器实例
    int blockSize;                  // 每个迷宫区块的像素大小
    QPushButton *solveButton;
    std::vector<point> solvedPath;
};
#endif // MAINWINDOW_H
