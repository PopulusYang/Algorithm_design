#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "backtrack_find_clue.h"
#include <QInputDialog>
#include <algorithm>
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTableWidget>       // 新增：用于创建表格
#include <QHeaderView>        // 新增：用于美化表头
#include <QCryptographicHash> // 新增：用于计算 SHA256
#include <QLabel>             // 修复：用于QLabel类型
#include <windows.h>

MonsterRenderThread::MonsterRenderThread(QObject *parent)
    : QThread(parent)
{
    monsterSheet.load("../img/monster.png");
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
                                        "大小 (7-51, 建议为奇数):", 11, 7, 51, 1, &ok);
    if (!ok)
    {
        mazeSize = 11; // 用户取消时的默认值
    }

    // 初始化并生成迷宫
    gameController = new GameController(mazeSize);
    gameController->generate();
    gameController->placeFeatures();

    autoCtrl.mazeinformation = gameController;

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
    Player.moveSpeed = 0.38f;
    Player.playerTimer = new QTimer(this);
    connect(Player.playerTimer, &QTimer::timeout, this, &MainWindow::ontimeout);
    connect(&Player, &player::needUpdate,
            this, QOverload<>::of(&QWidget::update));
    Player.playerTimer->start(16); // ~60fps
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法
    gameController->print();
    // 加载精灵图
    Player.playerSprite.load("../img/player.png"); // 确保player.png在资源文件或同目录下

    monsterThread = new MonsterRenderThread(this);
    connect(monsterThread, &MonsterRenderThread::frameReady, this, [this]()
            {
        QMutexLocker locker(&monsterFrameMutex);
        monsterFrameReady = monsterThread->getResult();
        update(); });
    monsterThread->start();

    solveButton = new QPushButton("打开控制面板", this);
    solveButton->setGeometry(10, 50, 100, 30);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::createAutoControlPanel);

    solveButton = new QPushButton("带我去找线索", this);
    solveButton->setGeometry(10, 90, 100, 30);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::drawCluePath);

    // 写一个按钮：按下弹出一个窗口，展示在当前识别进度下（根据receive_clue中的数据和数据个数决定）搜索出来的密码
    // 如有一个线索，就剩100种可能，两个线索，就10种可能，三个线索，就一种可能。
    // 根据当前的线索把所有可能的密码显示出来
    // 访问receive_clue：用gamecontroller->receive_clue访问。
    solveButton = new QPushButton("破解密码", this);
    solveButton->setGeometry(10, 130, 100, 30);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::crackPassword);

    solveButton = new QPushButton("输入密码", this);
    solveButton->setGeometry(10, 170, 100, 30);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::locker_status);

    autoThread = new std::thread([this]()
                                 { autoCtrl.thread_auto_run(); });
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
    if (runalongThread && runalongThread->joinable())
        runalongThread->join();
    delete runalongThread;
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
    QPixmap wallpixmap("../img/wall.png");

    for (int i = 0; i < mazeSize; ++i)
    {
        for (int j = 0; j < mazeSize; ++j)
        {
            QRect blockRect(j * blockSize, i * blockSize, blockSize, blockSize);
            MAZE blockType = static_cast<MAZE>(maze[i][j]);

            if (blockType == MAZE::WALL)
            {

                painter.drawPixmap(blockRect, wallpixmap);
                // painter.fillRect(blockRect, Qt::black);
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
                        QPixmap startPixmap("../img/start.png"); // 路径根据实际情况调整
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
                    else if (featureText == "E")
                    {
                        QPixmap exitPixmap("../img/exit.png"); // 路径根据实际情况调整
                        if (!exitPixmap.isNull())
                        {
                            QPixmap scaledexit = exitPixmap.scaled(centerSubRect.size() * 1.618, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledexit.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledexit.height()) / 2;
                            painter.drawPixmap(x, y, scaledexit);
                        }
                    }
                    else if (featureText == "G")
                    {
                        QPixmap goldPixmap("../img/gold.png"); // 路径根据实际情况调整
                        if (!goldPixmap.isNull())
                        {
                            QPixmap scaledgold = goldPixmap.scaled(centerSubRect.size() * 2.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledgold.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledgold.height()) / 2;
                            painter.drawPixmap(x, y, scaledgold);
                        }
                    }
                    else if (featureText == "L")
                    {
                        QPixmap lockerPixmap("../img/locker.png"); // 路径根据实际情况调整
                        if (!lockerPixmap.isNull())
                        {
                            QPixmap scaledlocker = lockerPixmap.scaled(centerSubRect.size() * 1.618, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledlocker.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledlocker.height()) / 2;
                            painter.drawPixmap(x, y, scaledlocker);
                        }
                    }
                    else if (featureText == "C")
                    {
                        QPixmap cluePixmap("../img/clue.png"); // 路径根据实际情况调整
                        if (!cluePixmap.isNull())
                        {
                            QPixmap scaledclue = cluePixmap.scaled(centerSubRect.size() * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            int x = centerSubRect.x() + (centerSubRect.width() - scaledclue.width()) / 2;
                            int y = centerSubRect.y() + (centerSubRect.height() - scaledclue.height()) / 2;
                            painter.drawPixmap(x, y, scaledclue);
                        }
                    }
                    else if (featureText == "T")
                    {
                        QPixmap trapPixmap("../img/trap.png"); // 路径根据实际情况调整
                        if (!trapPixmap.isNull())
                        {
                            QPixmap scaledtrap = trapPixmap.scaled(centerSubRect.size() * 3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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

    // 绘制玩家到三条线索的路径
    if (!cluePath.empty())
    {
        painter.setBrush(QBrush(QColor(0, 200, 255, 128))); // 半透明蓝色
        painter.setPen(Qt::NoPen);
        for (const auto &path : cluePath)
        {
            for (const auto &p : path)
            {
                QRect pathRect(p.second * blockSize + subBlockSize,
                               p.first * blockSize + subBlockSize,
                               subBlockSize, subBlockSize);
                painter.drawRect(pathRect);
            }
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
    if (runalongThread)
        return;
    if (gameController)
    {
        solvedPath = gameController->findBestPath();
        update(); // 触发重绘以显示路径
    }
    // 开始自动走
    runalongThread = new std::thread([this]()
                                     { autoCtrl.runalongthePath(solvedPath); });
}

void MainWindow::createAutoControlPanel()
{
    autoPanel = new AutoControlPanel(&autoCtrl);
    autoPanel->setAttribute(Qt::WA_DeleteOnClose); // 窗口关闭时自动销毁
    autoPanel->show();                             // 显示非模态窗口
}

void MainWindow::drawCluePath()
{
    std::pair<int, int> player_current_pos;
    player_current_pos.first = Player.playerPos.y() + 1;
    player_current_pos.second = Player.playerPos.x() + 1;
    clue_finder finder(gameController->getSize(), gameController->getmaze(), player_current_pos, 3);
    cluePath = finder.find_all_clue_paths();
    update(); // 触发重绘以显示路径
}

void MainWindow::generatePasswords_Backtracking(
    const int totalDigits,
    const std::map<int, int> &known_digits,
    QList<QPair<QString, QString>> &passwordHashes,
    QString currentPassword)
{
    // --- 基础情况：密码已达到所需长度 ---
    if (currentPassword.length() == totalDigits)
    {
        // 将密码字符串转换为字节数组以进行哈希计算
        QByteArray dataToHash = currentPassword.toUtf8();
        // 计算 SHA256 哈希值，并转换为十六进制字符串
        QString hash = QCryptographicHash::hash(dataToHash, QCryptographicHash::Sha256).toHex();
        // 将密码和哈希值添加到结果列表
        passwordHashes.append({currentPassword, hash});
        return;
    }

    // --- 递归步骤：构建密码的下一位 ---
    int next_pos_index = currentPassword.length() + 1; // 密码位置从1开始

    // 检查当前位置是否有已知数字
    auto it = known_digits.find(next_pos_index);
    if (it != known_digits.end())
    {
        // 如果该位数字已知，则直接使用该数字继续递归
        int known_digit = it->second;
        generatePasswords_Backtracking(totalDigits, known_digits, passwordHashes, currentPassword + QString::number(known_digit));
    }
    else
    {
        // 如果该位数字未知，则尝试所有可能的数字 (0-9)
        for (int i = 0; i < 10; ++i)
        {
            generatePasswords_Backtracking(totalDigits, known_digits, passwordHashes, currentPassword + QString::number(i));
        }
    }
}

void MainWindow::crackPassword()
{
    if (gameController->received_clue.empty())
    {
        QMessageBox::information(this, "提示", "没有线索可供破解密码");
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("破解结果");
    msgBox.setIcon(QMessageBox::Information);

    QString clueMsg = "已获得线索：\n";
    int idx = 1;
    std::map<int, int> known_digits;
    for (const auto &clue : gameController->received_clue)
    {
        clueMsg += QString("%1. 密码第%2位 = %3\n").arg(idx++).arg(clue.gen_order_index).arg(clue.password_dig_val);
        known_digits[clue.gen_order_index] = clue.password_dig_val;
    }
    msgBox.setText(clueMsg);

    msgBox.addButton("关闭", QMessageBox::RejectRole);
    QPushButton *showPossibleButton = msgBox.addButton("查看所有可能性", QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == showPossibleButton)
    {
        // --- 从这里开始是主要修改 ---

        // 5. 生成可能的密码及其 SHA256 哈希值
        // 使用一个列表来存储密码和哈希值的配对
        QList<QPair<QString, QString>> passwordHashes;

        const int totalDigits = 3;

        // ===================================================================
        // 使用回溯法代替穷举法
        // ===================================================================
        generatePasswords_Backtracking(totalDigits, known_digits, passwordHashes, "");
        // ===================================================================

        // 6. 在一个新的带表格的窗口中显示结果
        QDialog *possibleDialog = new QDialog(this);
        possibleDialog->setWindowTitle("可能的密码及SHA256值 (回溯法)");
        possibleDialog->setMinimumSize(650, 600); // 调整窗口大小以容纳表格

        // 创建表格控件
        QTableWidget *tableWidget = new QTableWidget(passwordHashes.size(), 2, possibleDialog);
        tableWidget->setHorizontalHeaderLabels({"密码 (Password)", "SHA256 哈希值 (Hash)"});
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 设置为只读
        tableWidget->verticalHeader()->setVisible(false);                // 隐藏行号

        // 填充表格数据
        int row = 0;
        for (const auto &pair : passwordHashes)
        {
            QTableWidgetItem *passwordItem = new QTableWidgetItem(pair.first);
            QTableWidgetItem *hashItem = new QTableWidgetItem(pair.second);

            // 居中显示，增加可读性
            passwordItem->setTextAlignment(Qt::AlignCenter);
            hashItem->setTextAlignment(Qt::AlignCenter);

            tableWidget->setItem(row, 0, passwordItem);
            tableWidget->setItem(row, 1, hashItem);
            row++;
        }

        // 让列宽自动适应内容
        tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive); // 第一列可手动调整
        tableWidget->resizeColumnsToContents();

        // 布局管理
        QVBoxLayout *layout = new QVBoxLayout(possibleDialog);
        layout->addWidget(tableWidget);

        QPushButton *closeButton = new QPushButton("关闭", possibleDialog);
        connect(closeButton, &QPushButton::clicked, possibleDialog, &QDialog::accept);
        layout->addWidget(closeButton);

        possibleDialog->setLayout(layout);
        possibleDialog->exec();

        delete possibleDialog;
    }
}

bool MainWindow::locker_status()
{
    // 1. 判断是否在储物柜附近
    if (gameController->is_near_locker)
    {
        // 立刻重置状态，避免重复触发
        gameController->is_near_locker = false;

        // 2. 获取正确密码并计算其SHA256哈希值
        int ans_password = gameController->getpassword();

        // 将int密码格式化为三位数的字符串（例如 42 -> "042"），以确保哈希值一致性
        QString passwordStr = QString("%1").arg(ans_password, 3, 10, QChar('0'));
        QByteArray dataToHash = passwordStr.toUtf8();
        QString sha256_hash = QCryptographicHash::hash(dataToHash, QCryptographicHash::Sha256).toHex();

        // 3. 创建一个自定义对话框作为弹窗
        QDialog dialog(this);
        dialog.setWindowTitle("储物柜密码");

        // 创建布局和控件
        QVBoxLayout *layout = new QVBoxLayout(&dialog);

        // 用于显示哈希值和提示信息的标签
        QLabel *infoLabel = new QLabel(&dialog);
        infoLabel->setText(QString("目标储物柜密码 (SHA256):\n%1\n\n请输入你猜测的3位密码:").arg(sha256_hash));
        infoLabel->setWordWrap(true); // 允许文本换行

        // 用于输入密码的行编辑器
        QLineEdit *passwordInput = new QLineEdit(&dialog);
        passwordInput->setPlaceholderText("例如: 123");
        // 使用验证器，只允许用户输入0-999之间的整数
        passwordInput->setValidator(new QIntValidator(0, 999, &dialog));
        passwordInput->setMaxLength(3); // 限制最大长度为3

        // 创建标准的 "OK" 和 "Cancel" 按钮
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept); // 连接OK按钮到accept槽
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject); // 连接Cancel按钮到reject槽

        // 将控件添加到布局中
        layout->addWidget(infoLabel);
        layout->addWidget(passwordInput);
        layout->addWidget(buttonBox);
        dialog.setLayout(layout);

        // 4. 显示对话框，并等待用户操作。代码会在此处暂停，直到对话框关闭。
        // dialog.exec() 返回 QDialog::Accepted (用户按了OK) 或 QDialog::Rejected (用户按了Cancel)
        if (dialog.exec() == QDialog::Accepted)
        {
            // 如果用户点击了 "OK"
            QString guessStr = passwordInput->text();

            // 将输入的字符串转换为整数
            bool conversion_ok;
            int guess_password = guessStr.toInt(&conversion_ok);

            // 5. 判断密码是否正确
            if (conversion_ok && guess_password == ans_password)
            {
                QMessageBox::information(this, "成功", "密码正确！储物柜已打开。");
                return true; // 密码正确，返回 true
            }
            else
            {
                QMessageBox::warning(this, "失败", "密码错误！");
                return false; // 密码错误，返回 false
            }
        }
        else
        {
            // 如果用户点击了 "Cancel" 或关闭了窗口
            return false; // 用户取消操作，返回 false
        }
    }
    else
    {
        // 如果不在储物柜附近（虽然代码里已经有这个，但为了逻辑完整性保留）
        gameController->is_near_locker = false;
        QMessageBox::information(this, "错误", "请前往密码锁处开锁。");
        return false; // 不在附近，自然无法成功打开
    }
}