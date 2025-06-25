#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QInputDialog>
#include <algorithm>
#include <QTimer>
#include <QKeyEvent>

MonsterRenderThread::MonsterRenderThread(QObject *parent)
    : QThread(parent)
{
    monsterSheet.load("../monster.png");
}

void MonsterRenderThread::requestFrame(int anim_, const QRect &srcRect_, const QSize &targetSize_)
{
    QMutexLocker locker(&mutex);
    anim = anim_;
    srcRect = srcRect_;
    targetSize = targetSize_;
    frameRequested = true;
    cond.wakeOne();
}

QPixmap MonsterRenderThread::getResult()
{
    QMutexLocker locker(&mutex);
    return result;
}

void MonsterRenderThread::run()
{
    while (running)
    {
        mutex.lock();
        if (!frameRequested)
            cond.wait(&mutex);
        if (!running)
        {
            mutex.unlock();
            break;
        }
        QRect r = srcRect;
        QSize sz = targetSize;
        frameRequested = false;
        mutex.unlock();

        QPixmap frame = monsterSheet.copy(r);
        QPixmap scaled = frame.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        mutex.lock();
        result = scaled;
        mutex.unlock();
        emit frameReady();
    }
}

void MonsterRenderThread::stop()
{
    QMutexLocker locker(&mutex);
    running = false;
    cond.wakeOne();
}

void MainWindow::ontimeout()
{
    emit needMove(gameController);
}

// MainWindow 构造函数中初始化
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
    connect(this, &MainWindow::needMove, &Player, &player::onPlayerMove);
    // 玩家初始位置在起点
    Player.playerPos = QPointF(gameController->start.y, gameController->start.x);
    Player.playerVel = QPointF(0, 0);
    Player.playerAcc = QPointF(0, 0);
    Player.inertia = 0.85f;
    Player.moveSpeed = 0.18f;
    Player.playerTimer = new QTimer(this);
    connect(Player.playerTimer, &QTimer::timeout, this, &MainWindow::ontimeout);
    connect(&Player, &player::needUpdate,
            this, QOverload<>::of(&QWidget::update));
    Player.playerTimer->start(16); // ~60fps
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法
    gameController->print();
    // 加载精灵图
    Player.playerSprite.load("../player.png"); // 确保player.png在资源文件或同目录下

    monsterThread = new MonsterRenderThread(this);
    connect(monsterThread, &MonsterRenderThread::frameReady, this, [this]()
            {
        QMutexLocker locker(&monsterFrameMutex);
        monsterFrameReady = monsterThread->getResult();
        update(); });
    monsterThread->start();

    solveButton = new QPushButton("打开控制面板", this);
    solveButton->setGeometry(10, 50, 150, 30);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::createAutoControlPanel);

    autoThread = new std::thread([this]()
                                 { autoCtrl.thread_auto_run(Player); });
}

MainWindow::~MainWindow()
{
    monsterThread->stop();
    monsterThread->wait();
    delete monsterThread;

    delete ui;
    delete gameController; // 释放内存
    autoCtrl.stopautocontrol();
    if (autoThread && autoThread->joinable())
        autoThread->join();
    delete autoThread;
    delete autoPanel;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    Player.pressedKeys.insert(event->key());
    event->accept();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    Player.pressedKeys.remove(event->key());
    event->accept();
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::InputMethod)
        return true; // 屏蔽输入法
    return QMainWindow::event(event);
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
    float camX = Player.playerPos.x() * blockSize - width() / 3.0f;
    float camY = Player.playerPos.y() * blockSize - height() / 3.0f;
    painter.translate(-camX, -camY);

    int mazeSize = gameController->getSize();
    const int (*maze)[MAXSIZE] = gameController->getMaze();
    const int subBlockSize = blockSize / 3;
    QPixmap wallpixmap("../wall.png");

    for (int i = 0; i < mazeSize; ++i)
    {
        for (int j = 0; j < mazeSize; ++j)
        {
            QRect blockRect(j * blockSize, i * blockSize, blockSize, blockSize);
            MAZE blockType = static_cast<MAZE>(maze[i][j]);
            

            if (blockType == MAZE::WALL)
            {   
                
                painter.drawPixmap(blockRect,wallpixmap);
                //painter.fillRect(blockRect, Qt::black);
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
                case MAZE::CLUE:
                    featureText = "C";
                    break;
                default:
                    featureText = "?";
                    break;
                }

                if (!featureText.isEmpty())
                {
                    if (featureText == "S")
                    {
                        QPixmap startPixmap("../start.png"); // 路径根据实际情况调整
                        if (!startPixmap.isNull())
                        {
                            // 缩放到中心子块大小
                            QPixmap scaledPixmap = startPixmap.scaled(centerSubRect.size() * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            // 计算居中位置
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledPixmap.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledPixmap.height()) / 2;
                            painter.drawPixmap(x, y, scaledPixmap);
                        }
                    }
                    else if (featureText == "B")
                    {
                        int frameW = 5120 / 8;
                        int frameH = 640;
                        int frameIdx = bossAnim % 8;
                        QRect srcRect(frameIdx * frameW, 0, frameW, frameH);
                        QSize targetSize(centerSubRect.size() * 2);

                        monsterThread->requestFrame(bossAnim, srcRect, targetSize);

                        QMutexLocker locker(&monsterFrameMutex);
                        if (!monsterFrameReady.isNull())
                        {
                            int x = centerSubRect.x() + (centerSubRect.width() - monsterFrameReady.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - monsterFrameReady.height()) / 2;
                            painter.drawPixmap(x, y, monsterFrameReady);
                        }
                    }
                    else if(featureText == "E"){
                        QPixmap exitPixmap("../exit.png"); // 路径根据实际情况调整
                        if (!exitPixmap.isNull()){
                            QPixmap scaledexit = exitPixmap.scaled(centerSubRect.size()*1.618, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledexit.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledexit.height()) / 2;
                            painter.drawPixmap(x, y, scaledexit);
                        }
                    }
                    else if(featureText == "G"){
                         QPixmap goldPixmap("../gold.png"); // 路径根据实际情况调整
                        if (!goldPixmap.isNull()){
                            QPixmap scaledgold = goldPixmap.scaled(centerSubRect.size()*2.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledgold.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledgold.height()) / 2;
                            painter.drawPixmap(x, y, scaledgold);
                        }
                    }
                    else if(featureText == "L"){
                         QPixmap lockerPixmap("../locker.png"); // 路径根据实际情况调整
                        if (!lockerPixmap.isNull()){
                            QPixmap scaledlocker = lockerPixmap.scaled(centerSubRect.size()*1.618, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledlocker.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledlocker.height()) / 2;
                            painter.drawPixmap(x, y, scaledlocker);
                        }
                    }
                    else if(featureText == "C"){
                         QPixmap cluePixmap("../clue.png"); // 路径根据实际情况调整
                        if (!cluePixmap.isNull()){
                            QPixmap scaledclue = cluePixmap.scaled(centerSubRect.size()*2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledclue.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledclue.height()) / 2;
                            painter.drawPixmap(x, y, scaledclue);
                        }
                    }
                    else if(featureText == "T"){
                         QPixmap trapPixmap("../trap.png"); // 路径根据实际情况调整
                        if (!trapPixmap.isNull()){
                            QPixmap scaledtrap = trapPixmap.scaled(centerSubRect.size()*3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledtrap.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledtrap.height()) / 2;
                            painter.drawPixmap(x, y, scaledtrap);
                        }
                    }
                    else
                    {
                        painter.setPen(Qt::black);
                        painter.drawText(centerSubRect, Qt::AlignCenter, featureText);
                    }
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
    int frameW = 600 / 10;      // 10列
    int frameH = 290 / 4;       // 4行
    int dir = Player.playerDir; // 0:左 1:下 2:上 3:右
    int col = 0;
    if (Player.playerState == "idle")
        col = 0 + (Player.playerAnim % 2);
    else if (Player.playerState == "walk")
        col = 2 + (Player.playerAnim % 4);
    else if (Player.playerState == "attack")
        col = 6 + (Player.playerAnim % 4);

    QRect srcRect(col * frameW, dir * frameH, frameW, frameH);

    int px = Player.playerPos.x() * blockSize + blockSize * 3 / 8;
    int py = Player.playerPos.y() * blockSize + blockSize * 3 / 8;
    int size = blockSize / 2; // 玩家显示大小

    painter.drawPixmap(QRect(px, py, size, size), Player.playerSprite, srcRect);

    // 添加渐变遮罩
    painter.restore();
    painter.setRenderHint(QPainter::Antialiasing, true);

    bossAnimFrameCounter++;
    if (bossAnimFrameCounter >= 16)
    { // 每8帧切换一次动画帧
        bossAnim = (bossAnim + 1) % 8;
        bossAnimFrameCounter = 0;
    }

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

void MainWindow::createAutoControlPanel()
{
    autoPanel = new AutoControlPanel(&autoCtrl);
    autoPanel->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时自动销毁
    autoPanel->show();                             // 显示非模态窗口
}
