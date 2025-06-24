#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QInputDialog>
#include <algorithm>

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

    solveButton = new QPushButton("解密迷宫", this);
    solveButton->setGeometry(10, 10, 100, 30);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveMazeClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete gameController; // 释放内存
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QPainter painter(this);

    if (!gameController)
    {
        return;
    }

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
}

void MainWindow::onSolveMazeClicked()
{
    if (gameController)
    {
        solvedPath = gameController->findBestPath();
        update(); // 触发重绘以显示路径
    }
}
