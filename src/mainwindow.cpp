#include "heads/lock.h"       // 添加 ock.h 头文件
#include <QCryptographicHash> // 新增：用于计算 SHA256
#include <QHeaderView>        // 新增：用于美化表头
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel> // 修复：用于QLabel类型
#include <QMessageBox>
#include <QTableWidget> // 新增：用于创建表格
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <random> // 新增：用于屏幕抖动
#include <windows.h>
#include<QMessageBox>
#include "heads/mainwindow.h"
#include "./ui_mainwindow.h"
#include "heads/backtrack_find_clue.h"

void MainWindow::ontimeout()
{
    emit needMove(gameController);
}

// MainWindow 构造函数中初始化
MainWindow::MainWindow(int mazeSize, int model, gamemain *informations,
                       QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_model = model;
    // 初始化并生成迷宫
    if (model == 2)
    {
        gameController = new GameController(mazeSize);
    }
    else
    {
        gameController = new GameController(informations);
    }

    autoCtrl.mazeinformation = gameController;

    const int maxWidth = 1280;
    const int maxHeight = 720;
    // 使用gameController->getSize()以获取经过验证的实际大小
    blockSize = std::min(maxWidth / gameController->getSize(),
                         maxHeight / gameController->getSize());
    blockSize -= (blockSize % 3); // 确保blockSize可以被3整除，以适应3x3子网格

    // 根据迷宫尺寸和区块大小，设置窗口的固定大小
    int width = gameController->getSize() * blockSize;
    int height = gameController->getSize() * blockSize;
    this->setFixedSize(width, height);

    generationTimer = new QTimer(this);
    connect(generationTimer, &QTimer::timeout, this,
            &MainWindow::onGenerationStep);
    if (model == 2)
    {
        gameController->generate_init();
    }
    generationTimer->start(5); // 设置地图生成动画5ms刷新一次

    solveButton = new QPushButton("一键开挂", this);
    solveButton->setGeometry(10, 10, 100, 30);
    connect(solveButton, &QPushButton::clicked, this,
            &MainWindow::onSolveMazeClicked);

    exitButton = new QPushButton("返回主界面", this);
    exitButton->setGeometry(10, 50, 100, 30);
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::onExitClicked);

    connect(this, &MainWindow::needMove, &Player, &player::onPlayerMove);
    connect(&Player, &player::trapTriggered, this,
            &MainWindow::onTrapTriggered);

    connect(&Player, &player::exitReached, this, &MainWindow::onExitReached);

    Player.playerVel = QPointF(0, 0);
    Player.playerAcc = QPointF(0, 0);
    Player.moveSpeed = 0.5f;
    Player.playerTimer = new QTimer(this);
    connect(Player.playerTimer, &QTimer::timeout, this, &MainWindow::ontimeout);
    connect(&Player, &player::needUpdate, this,
            QOverload<>::of(&QWidget::update));
    Player.playerTimer->start(16); // ~60fps
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, false); // 禁用输入法

    // 初始化并启动新的渲染线程
    m_renderThread = new RenderThread(this);
    connect(m_renderThread, &RenderThread::frameReady, this,
            &MainWindow::onFrameReady);
    m_renderThread->start();

    // 初始化用于驱动渲染的计时器
    m_renderTimer = new QTimer(this);
    connect(m_renderTimer, &QTimer::timeout, this, &MainWindow::onRenderTick);
    m_renderTimer->start(16); // ~60 FPS

    // crackPassword locker_status改为用E建触发

    autoThread = new std::thread([this]()
                                 { autoCtrl.thread_auto_run(); });
}

void MainWindow::onExitReached()
{
    // 停止当前窗口的所有计时器，以防止后台继续处理
    Player.playerTimer->stop();
    if (generationTimer)
    {
        generationTimer->stop();
    }
    if (m_renderTimer)
    {
        m_renderTimer->stop();
    }

    // 停止线程
    autoCtrl.stopautocontrol();
    if (autoThread && autoThread->joinable())
    {
        autoThread->join();
    }
    if (runalongThread && runalongThread->joinable())
    {
        if (autoCtrl.rundone)
            runalongThread->join();
    }
    if(m_model==2)
    {
        QMessageBox::information(this,"提示","胜利");
    }
    if (gameController->bosshp.size()>0)
    {
        auto [i, p] = get_crack_info();

        // 创建一个新的对话框来显示胜利信息
        victoryDialog = new QDialog(nullptr);
        victoryDialog->setWindowTitle("密码已破译");

        QVBoxLayout *layout = new QVBoxLayout(victoryDialog);

        QLabel *titleLabel = new QLabel("密码已经破译，即将boss大战", victoryDialog);
        titleLabel->setFont(QFont("Arial", 16, QFont::Bold));
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        // 计算并显示最短路径信息
        // solvedPath = gameController->findBestPath({gameController->start.y, gameController->start.x});
        QString pathText;
        if (!solvedPath.empty())
        {
            pathText = QString("最短路径长度: %1\n路径点: ").arg(solvedPath.size());
            for (const auto &point : solvedPath)
            {
                pathText += QString("(%1,%2) ").arg(point.x).arg(point.y);
            }
        }
        else
        {
            pathText = "未能计算出路径。";
        }

        QTextEdit *infoText = new QTextEdit(victoryDialog);
        infoText->setReadOnly(true);
        infoText->setText(QString("<b>通关统计:</b><br><br>"
                                  "解密尝试次数: %1<br>"
                                  "最终破译密码: %2<br><br>"
                                  "%3")
                              .arg(i)
                              .arg(QString::fromStdString(p))
                              .arg(pathText));
        layout->addWidget(infoText);

        QPushButton *okButton = new QPushButton("继续", victoryDialog);
        connect(okButton, &QPushButton::clicked, victoryDialog, &QDialog::accept);
        layout->addWidget(okButton);

        victoryDialog->setLayout(layout);
        victoryDialog->exec(); // 显示对话框并等待用户交互


        // 创建并显示新的boss窗口
        boss *bossWindow = new boss(gameController->bosshp,
                                    gameController->Skills,Player.playersource - i); // 创建 boss 窗口的实例
        bossWindow->show();                                  // 显示它

        // 关闭当前的迷宫窗口
        this->close();
    }
}

void MainWindow::onExitClicked()
{
    this->hide();
    emit exit_mainwindow();
}
MainWindow::~MainWindow()
{
    if (generationTimer)
    {
        generationTimer->stop();
        delete generationTimer;
    }

    // 停止并清理渲染线程
    if (m_renderThread)
    {
        m_renderThread->stop();
        m_renderThread->wait();
        delete m_renderThread;
    }

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
    if (Player.pressedKeys.contains(event->key()))
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

    // paintEvent 现在非常简单，只绘制最新渲染好的帧
    QMutexLocker locker(&m_frameMutex);
    if (!m_lastFrame.isNull())
    {
        painter.drawPixmap(0, 0, m_lastFrame);
    }
}

void MainWindow::onRenderTick()
{
    if (!gameController || !m_renderThread)
        return;

    // 更新屏幕抖动
    if (m_screenShakeFrames > 0)
    {
        m_screenShakeFrames--;
    }

    if (!Player.ai_control && Player.cando && Player.pressedKeys.contains(Qt::Key_E))
    {
        int playerTileX = qRound(Player.playerPos.y() + 0.15);
        int playerTileY = qRound(Player.playerPos.x() + 0.1);
        switch (gameController->maze[playerTileX][playerTileY])
        {
        case static_cast<int>(MAZE::CLUE):
            record_clue(gameController, playerTileX, playerTileY);
            crackPassword();
            break;
        case static_cast<int>(MAZE::EXIT):
        case static_cast<int>(MAZE::LOCKER):
            bool istrue = locker_status();
            if (gameController->maze[playerTileX][playerTileY] == static_cast<int>(MAZE::EXIT))
            {
                if (istrue)
                    onExitReached();
            }
        }
        Player.pressedKeys.remove(Qt::Key_E);
    }

    // 更新伤害指示器
    for (auto it = m_damageIndicators.begin(); it != m_damageIndicators.end();)
    {
        it->lifetime--;
        if (it->lifetime <= 0)
        {
            it = m_damageIndicators.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 更新Boss动画帧
    bossAnimFrameCounter++;
    if (bossAnimFrameCounter >= 8)
    {                                  // 数值越大动画越慢
        bossAnim = (bossAnim + 1) % 8; // 总共有8帧
        bossAnimFrameCounter = 0;
    }

    // 更新金币动画帧
    goldAnimFrameCounter++;
    if (goldAnimFrameCounter >= 8)
    {                                   // 数值越大动画越慢
        goldAnim = (goldAnim + 1) % 18; // 总共有18帧
        goldAnimFrameCounter = 0;
    }

    // 更新线索动画帧
    clueAnimFrameCounter++;
    if (clueAnimFrameCounter >= 8)
    {                                  // 数值越大动画越慢
        clueAnim = (clueAnim + 1) % 8; // 总共有8帧
        clueAnimFrameCounter = 0;
    }

    // 1. 收集所有需要渲染的数据
    SceneData data;
    data.gameController = gameController; // 直接传递指针
    data.solvedPath = solvedPath;
    data.playerData = &Player; // 复制玩家状态
    data.blockSize = blockSize;
    data.isGenerating = (generationTimer != nullptr);
    data.windowSize = this->size();
    data.bossAnim = this->bossAnim;
    data.goldAnim = this->goldAnim;
    data.clueAnim = this->clueAnim;
    data.damageIndicators = &m_damageIndicators; // 传递伤害指示器
    const float baseBlockSize = 63.0f;
    float dynamicScale = baseBlockSize / static_cast<float>(blockSize);
    data.camX = Player.playerPos.x() * blockSize - (width() / 3.0f) / dynamicScale;
    data.camY = Player.playerPos.y() * blockSize - (height() / 3.0f) / dynamicScale;

    // 应用屏幕抖动
    if (m_screenShakeFrames > 0)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-5.0, 5.0);
        data.camX += dis(gen);
        data.camY += dis(gen);
    }

    // 2. 请求渲染线程绘制一帧
    m_renderThread->requestFrame(data);
}

void MainWindow::onFrameReady(const QPixmap &frame)
{
    // 3. 收到渲染好的帧，保存它并请求UI更新
    {
        QMutexLocker locker(&m_frameMutex);
        m_lastFrame = frame;
    }
    update(); // 触发 paintEvent
}

void MainWindow::onSolveMazeClicked()
{
    if (runalongThread)
        return;
    if (gameController)
    {
        solvedPath = gameController->findBestPath(
            {static_cast<int>(std::round(Player.playerPos.y())),
             static_cast<int>(std::round(Player.playerPos.x()))});
        update(); // 触发重绘以显示路径
    }
    // 开始自动走
    Player.ai_control = true;
    runalongThread = new std::thread([this]()
                                     { autoCtrl.runalongthePath(solvedPath); });
}

void MainWindow::onTrapTriggered(const QPointF &playerPos)
{
    m_screenShakeFrames = 15;                             // 抖动15帧
    m_damageIndicators.push_back({"-20", playerPos, 60}); // 显示60帧
}

void MainWindow::generatePasswords_Backtracking(
    const int totalDigits, const std::map<int, int> &known_digits,
    QList<QPair<QString, QString>> &passwordHashes, QString currentPassword)
{
    // --- 基础情况：密码已达到所需长度 ---
    if (currentPassword.length() == totalDigits)
    {
        // 将密码字符串转换为字节数组以进行哈希计算
        QByteArray dataToHash = currentPassword.toUtf8();
        // 计算 SHA256 哈希值，并转换为十六进制字符串
        QString hash = QCryptographicHash::hash(dataToHash, QCryptographicHash::Sha256)
                           .toHex();
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
        generatePasswords_Backtracking(
            totalDigits, known_digits, passwordHashes,
            currentPassword + QString::number(known_digit));
    }
    else
    {
        // 如果该位数字未知，则尝试所有可能的数字 (0-9)
        for (int i = 0; i < 10; ++i)
        {
            generatePasswords_Backtracking(
                totalDigits, known_digits, passwordHashes,
                currentPassword + QString::number(i));
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
        clueMsg += QString("%1. 密码第%2位 = %3\n")
                       .arg(idx++)
                       .arg(clue.gen_order_index)
                       .arg(clue.password_dig_val);
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
        generatePasswords_Backtracking(totalDigits, known_digits,
                                       passwordHashes, "");
        // ===================================================================

        // 6. 在一个新的带表格的窗口中显示结果
        QDialog *possibleDialog = new QDialog(this);
        possibleDialog->setWindowTitle("可能的密码及SHA256值 (回溯法)");
        possibleDialog->setMinimumSize(650, 600); // 调整窗口大小以容纳表格

        // 创建表格控件
        QTableWidget *tableWidget = new QTableWidget(passwordHashes.size(), 2, possibleDialog);
        tableWidget->setHorizontalHeaderLabels(
            {"密码 (Password)", "SHA256 哈希值 (Hash)"});
        tableWidget->setEditTriggers(
            QAbstractItemView::NoEditTriggers);           // 设置为只读
        tableWidget->verticalHeader()->setVisible(false); // 隐藏行号

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
        tableWidget->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
        tableWidget->horizontalHeader()->setSectionResizeMode(
            0, QHeaderView::Interactive); // 第一列可手动调整
        tableWidget->resizeColumnsToContents();

        // 布局管理
        QVBoxLayout *layout = new QVBoxLayout(possibleDialog);
        layout->addWidget(tableWidget);

        QPushButton *closeButton = new QPushButton("关闭", possibleDialog);
        connect(closeButton, &QPushButton::clicked, possibleDialog,
                &QDialog::accept);
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

        // 2. 获取正确密码
        int ans_password = gameController->getpassword();
        QString passwordStr = QString("%1").arg(ans_password, 3, 10, QChar('0'));
        QByteArray dataToHash = passwordStr.toUtf8();
        QString sha256_hash = QCryptographicHash::hash(dataToHash, QCryptographicHash::Sha256).toHex();

        // 3. 创建并显示密码对话框
        Lock lockDialog(sha256_hash, gameController->received_clue, this);
        if (lockDialog.exec() == QDialog::Accepted)
        {
            int guess_password = lockDialog.getPassword();

            // 5. 判断密码是否正确
            if (guess_password == ans_password)
            {
                QMessageBox::information(this, "成功",
                                         "密码正确！储物柜已打开。");
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

void MainWindow::onGenerationStep()
{
    if (gameController && gameController->generateStep())
    {
        update();
    }
    else
    {
        if (generationTimer)
        {
            generationTimer->stop();
            delete generationTimer;
            generationTimer = nullptr;
        }

        if (gameController)
        {
            if (m_model == 2)
            {
                gameController->placeFeatures();       // Place features after
                                                       // generation is complete
                gameController->exportToJsonDefault(); // 导出到../map.json
            }
            gameController->print(); // 在此处调用 print
            // 在确定起点后设置玩家位置
            Player.playerPos = QPointF(gameController->start.y, gameController->start.x);
        }
        update();
    }
}
