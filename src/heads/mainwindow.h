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
#include <QDialogButtonBox>
#include "player.h"
#include "gamecontrol.h"
#include "autocontrol.h"
#include "renderthread.h"
#include "boss.h"
#include "autothread.h"
#include <thread>
#include <list>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(int mazeSize, int model, gamemain *informations = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override; // 用于关闭输入法
    void crackPassword();
    bool locker_status();
    void generatePasswords_Backtracking(
        const int totalDigits,
        const std::map<int, int> &known_digits,
        QList<QPair<QString, QString>> &passwordHashes,
        QString currentPassword);
    void paintEvent(QPaintEvent *event) override;

private slots:
    void ontimeout();
    void onGenerationStep();
    void onSolveMazeClicked();
    void onFrameReady(const QPixmap &frame);
    void onRenderTick();
    void onTrapTriggered(const QPointF &playerPos);
    void onExitReached();
    void onExitClicked();
    void exitbossgame();
    void onShowWarningMessageBox(const QString &title, const QString &text);
    void onShowInformationMessageBox(const QString &title, const QString &text);

private:
    Ui::MainWindow *ui;
    GameController *gameController;
    int blockSize;
    QPushButton *solveButton;
    QPushButton *exitButton;
    std::vector<point> solvedPath;
    QTimer *generationTimer = nullptr;
    boss *bossWindow = nullptr;
    player Player;
    bool gameover = false;
    // Boss 相关
    int bossAnim = 0;
    int bossAnimFrameCounter = 0;

    // Gold 相关
    int goldAnim = 0;
    int goldAnimFrameCounter = 0;

    // Clue 相关
    int clueAnim = 0;
    int clueAnimFrameCounter = 0;

    // 移除 MonsterRenderThread，用新的 RenderThread 代替
    RenderThread *m_renderThread = nullptr;
    QPixmap m_lastFrame;
    QMutex m_frameMutex;
    QTimer *m_renderTimer = nullptr; // 用于驱动渲染的计时器

    int m_screenShakeFrames = 0;
    int m_model = -1;
    std::list<DamageIndicator> m_damageIndicators;

    autocontroller autoCtrl = autocontroller(&Player);
    AutoThread *autoThread = nullptr;
    RunalongThread *runalongThread = nullptr;

signals:
    void needMove(GameController *gameController);
    void exit_mainwindow();
    void showWarningMessageBox(const QString &title, const QString &text);
    void showInformationMessageBox(const QString &title, const QString &text);
};
#endif // MAINWINDOW_H
