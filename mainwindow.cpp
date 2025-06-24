#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QInputDialog>
#include <algorithm>
#include <QTimer>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 添加一个选择地图大小的功能
    bool ok;
    int mazeSize = QInputDialog::getInt(this, "选择迷宫大小",
                                        "大小 (7-15, 建议为奇数):", 11, 7, 15, 1, &ok);
    if (!ok)
    {
        mazeSize = 11; // 用户取消时的默认值
    }

    // 初始化并生成迷宫
    gameController = new GameController(mazeSize);
    gameController->generate();
    gameController->placeFeatures();

    // 动态计算blockSize以适应1920*1080的屏幕
    const int maxWidth = 1280;
    const int maxHeight = 720;
    // 使用gameController->getSize()以获取经过验证的实际大小
    blockSize = std::min(maxWidth / gameController->getSize(), maxHeight / gameController->getSize());
    blockSize -= (blockSize % 3); // 确保blockSize可以被3整除，以适应3x3子网格

    // 根据迷宫尺寸和区块大小，设置窗口的固定大小
    int width = gameController->getSize() * blockSize;
    int height = gameController->getSize() * blockSize;
    this->setFixedSize(width, height);

    solveButton = new QPushButton("一键开挂", this);
    solveButton->setGeometry(10, 10, 100, 30);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveMazeClicked);

    // 玩家初始位置在起点
    playerPos = QPointF(gameController->start.y, gameController->start.x);
    playerVel = QPointF(0, 0);
    playerAcc = QPointF(0, 0);
    inertia = 0.85f;
    moveSpeed = 0.18f;
    playerTimer = new QTimer(this);
    connect(playerTimer, &QTimer::timeout, this, &MainWindow::onPlayerMove);
    playerTimer->start(16); // ~60fps
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法

    // 加载精灵图
    playerSprite.load("../player.png"); // 确保player.png在资源文件或同目录下
}

MainWindow::~MainWindow()
{
    delete ui;
    delete gameController; // 释放内存
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    pressedKeys.insert(event->key());
    event->accept();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    pressedKeys.remove(event->key());
    event->accept();
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::InputMethod)
        return true; // 屏蔽输入法
    return QMainWindow::event(event);
}

void MainWindow::onPlayerMove()
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
    int nx = qRound(nextPos.y()+0.15);
    int ny = qRound(nextPos.x()+0.1);
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

    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QPainter painter(this);
    if (!gameController)
        return;

    // --> 将摄像机中心移动到玩家附近
    painter.save();
    painter.scale(1.5, 1.5); // 将显示区域放大
    float camX = playerPos.x() * blockSize - width() / 3.0f;
    float camY = playerPos.y() * blockSize - height() / 3.0f;
    painter.translate(-camX, -camY);

    int mazeSize = gameController->getSize();
    const int (*maze)[MAXSIZE] = gameController->getMaze();
    const int subBlockSize = blockSize / 3;

    for (int i = 0; i < mazeSize; ++i)
    {
        for (int j = 0; j < mazeSize; ++j)
        {
            QRect blockRect(j * blockSize, i * blockSize, blockSize, blockSize);
            MAZE blockType = static_cast<MAZE>(maze[i][j]);

            if (blockType == MAZE::WALL)
            {
                painter.fillRect(blockRect, Qt::black);
            }
            else
            {
                // 为每个单元格绘制一个3x3的网格
                for (int sub_i = 0; sub_i < 3; ++sub_i)
                {
                    for (int sub_j = 0; sub_j < 3; ++sub_j)
                    {
                        QRect subRect(j * blockSize + sub_j * subBlockSize,
                                      i * blockSize + sub_i * subBlockSize,
                                      subBlockSize, subBlockSize);
                        painter.fillRect(subRect, Qt::white);
                        painter.setPen(Qt::lightGray);
                        painter.drawRect(subRect);
                    }
                }

                // 在中心子块中绘制特征文本
                QRect centerSubRect(j * blockSize + subBlockSize,
                                    i * blockSize + subBlockSize,
                                    subBlockSize, subBlockSize);

                QString featureText;
                switch (blockType)
                {
                case MAZE::START:
                    featureText = "S";
                    break;
                case MAZE::EXIT:
                    featureText = "E";
                    break;
                case MAZE::SOURCE:
                    featureText = "G";
                    break;
                case MAZE::TRAP:
                    featureText = "T";
                    break;
                case MAZE::LOCKER:
                    featureText = "L";
                    break;
                case MAZE::BOSS:
                    featureText = "B";
                    break;
                case MAZE::WAY:
                    // 普通通路，无需额外绘制
                    break;
                default:
                    featureText = "?";
                    break;
                }

                if (!featureText.isEmpty())
                {
                    painter.setPen(Qt::black);
                    painter.drawText(centerSubRect, Qt::AlignCenter, featureText);
                }
            }
        }
    }

    // 绘制路径
    if (!solvedPath.empty())
    {
        painter.setBrush(QBrush(QColor(0, 255, 0, 128))); // 半透明绿色
        painter.setPen(Qt::NoPen);
        for (const auto &p : solvedPath)
        {
            // 路径点是 (row, col)，对应于 (i, j)
            // 我们可以给中心子块上色来显示路径
            QRect pathRect(p.y * blockSize + subBlockSize,
                           p.x * blockSize + subBlockSize,
                           subBlockSize, subBlockSize);
            painter.drawRect(pathRect);
        }
    }

    // 绘制玩家
    // painter.setBrush(QBrush(Qt::red));
    // painter.setPen(Qt::NoPen);
    // int px = playerPos.x() * blockSize + blockSize * 3 / 8;
    // int py = playerPos.y() * blockSize + blockSize * 3 / 8;
    // int size = blockSize / 4; // 更小的玩家
    // painter.drawEllipse(QRect(px, py, size, size));

    // 使用精灵图绘制玩家
    int frameW = 600 / 10; // 10列
    int frameH = 290 / 4;  // 4行
    int dir = playerDir;   // 0:左 1:下 2:上 3:右
    int col = 0;
    if (playerState == "idle")
        col = 0 + (playerAnim % 2);
    else if (playerState == "walk")
        col = 2 + (playerAnim % 4);
    else if (playerState == "attack")
        col = 6 + (playerAnim % 4);

    QRect srcRect(col * frameW, dir * frameH, frameW, frameH);

    int px = playerPos.x() * blockSize + blockSize * 3 / 8;
    int py = playerPos.y() * blockSize + blockSize * 3 / 8;
    int size = blockSize / 2; // 玩家显示大小

    painter.drawPixmap(QRect(px, py, size, size), playerSprite, srcRect);

    // 添加渐变遮罩
    painter.restore();
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRadialGradient grad(rect().center(), rect().width() / 2, rect().center());
    grad.setColorAt(0, QColor(255, 255, 255, 0));
    grad.setColorAt(1, QColor(0, 0, 0, 150));
    painter.setBrush(grad);
    painter.drawRect(rect());
}

void MainWindow::onSolveMazeClicked()
{
    if (gameController)
    {
        solvedPath = gameController->findBestPath();
        update(); // 触发重绘以显示路径
    }
}
