#include "heads/gamechoose.h"
#include "ui_gamechoose.h"

gamechoose::gamechoose(QWidget *parent)
    : QWidget(parent), ui(new Ui::gamechoose)
{
    ui->setupUi(this);

    this->setWindowTitle("游戏选择界面");
    this->setFixedSize(693, 693);
    QLabel *titleLabel = new QLabel(this); // 指定父对象为当前窗口(this)

    // 2. 设置文本内容
    titleLabel->setText("欢迎来到迷宫游戏");
    titleLabel->setStyleSheet(
        "color: black;"                   // 字体颜色
        "font-size: 50px;"                // 字体大小
        "font-family: 'Microsoft YaHei';" // 字体类型
        "font-weight: bold;"              // 字体加粗
        "background-color: transparent;"  // 背景透明
    );

    // 4. 设置对齐方式 (水平居中)
    titleLabel->setAlignment(Qt::AlignCenter);
    int labelWidth = this->width();     // 标签宽度与窗口相同
    int labelHeight = 80;               // 标签高度
    int labelX = 0;                     // X 坐标 (从窗口左边缘开始)
    int labelY = this->height() * 0.10; // Y 坐标 (距离窗口顶部 15% 的位置)
    titleLabel->setGeometry(labelX, labelY, labelWidth, labelHeight);

    QLabel *promptLabel = new QLabel(this);
    promptLabel->setText("请选择迷宫生成方式");

    // 3. 设置 QLabel 的样式
    promptLabel->setStyleSheet(
        "color: black;"
        "font-size: 28px;" // 字体可以比主标题小一些
        "font-family: 'Microsoft YaHei';"
        "background-color: transparent;");
    promptLabel->setAlignment(Qt::AlignCenter);
    int labelWidth2 = this->width();
    int labelHeight2 = 80;
    int labelX2 = 0;                                    // X 坐标：窗口宽度的 25% 处
    int labelY2 = this->height() / 2 - labelHeight / 2; // Y 坐标：窗口高度的正中间
    promptLabel->setGeometry(labelX2, labelY2, labelWidth2, labelHeight2);

    QPushButton *fileButton = new QPushButton("文件读入", this);
    QPushButton *randomButton = new QPushButton("随机生成", this);
    QString buttonStyle =
        "QPushButton {"
        "    background-color: white;"   // 普通状态：白色背景
        "    border: 2px solid #DDDDDD;" // 边框：非常浅的灰色
        "    color: black;"              // 字体颜色：黑色
        "    padding: 15px 32px;"        // 内边距 (上下 15px, 左右 32px)
        "    text-align: center;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    margin: 4px 2px;"
        "    border-radius: 12px;" // 圆角大小
        "}"
        "QPushButton:hover {"
        "    background-color: #F0F0F0;" // 悬停状态：浅灰色背景
        "    border: 2px solid #CCCCCC;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #A9A9A9;" // 点击状态：深灰色背景
        "    border: 2px solid #999999;"
        "}";

    fileButton->setStyleSheet(buttonStyle);
    randomButton->setStyleSheet(buttonStyle);

    // 3. 设置按钮的位置
    // 定义按钮的尺寸和它们之间的间距
    int buttonWidth = 200;
    int buttonHeight = 60;
    int gap = 50; // 两个按钮之间的水平间距

    // Y 坐标：我们将它们放在提示标签的下方
    int buttonY = this->height() * 0.75 - (buttonHeight / 2);

    // X 坐标：计算使它们对称分布在窗口中心左右的位置
    int fileButtonX = (this->width() / 2) - (gap / 2) - buttonWidth;
    int randomButtonX = (this->width() / 2) + (gap / 2);

    fileButton->setGeometry(fileButtonX, buttonY, buttonWidth, buttonHeight);
    randomButton->setGeometry(randomButtonX, buttonY, buttonWidth, buttonHeight);
    connect(fileButton, &QPushButton::clicked, this, &gamechoose::onFileButtonClicked);

    // 将 randomButton 的 clicked() 信号连接到 onRandomButtonClicked() 槽
    connect(randomButton, &QPushButton::clicked, this, &gamechoose::onRandomButtonClicked);
}

gamechoose::~gamechoose()
{
    delete ui;
    delete w;
}

void gamechoose::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event); // 避免编译器关于未使用参数的警告

    QPainter painter(this);
    QString imagePath = QDir::currentPath() + "/img/background2.png";

    QPixmap pixmap(imagePath);

    if (pixmap.isNull())
    {
        qWarning() << "Failed to load background image:" << imagePath;
        return;
    }

    painter.drawPixmap(this->rect(), pixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void gamechoose::onFileButtonClicked()
{
    model = 1;
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择迷宫JSON文件",                               // 对话框标题
        "D:/Downloads/验收及测试文件 (2)/验收及测试文件", // 默认打开的目录（空表示上次的目录或默认目录）
        "JSON 文件 (*.json)"                              // 文件过滤器
    );

    // 如果用户没有选择文件（点击了取消），则直接返回
    if (filePath.isEmpty())
    {
        return;
    }

    // 2. 读取文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "错误", "无法打开文件: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // 3. 解析 JSON 数据
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData, &jsonError);

    if (jsonDoc.isNull())
    {
        QMessageBox::warning(this, "错误", "JSON解析失败: " + jsonError.errorString());
        return;
    }

    if (!jsonDoc.isObject())
    {
        QMessageBox::warning(this, "错误", "无效的JSON格式：顶层必须是一个对象。");
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 4. 处理 "maze" 数据
    if (jsonObj.contains("maze") && jsonObj["maze"].isArray())
    {
        QJsonArray mazeArray = jsonObj["maze"].toArray();
        int height = mazeArray.size();
        int width = 0;
        if (height > 0)
        {
            width = mazeArray[0].toArray().size();
        }

        // 检查迷宫尺寸是否超出最大限制
        if (height > MAXSIZE || width > MAXSIZE)
        {
            QMessageBox::warning(this, "错误", QString("迷宫尺寸 (%1x%2) 超出最大限制 (%3x%3)。").arg(height).arg(width).arg(MAXSIZE));
            return;
        }

        // 存入迷宫宽度
        this->mazesize = width;
        this->dimension = width;
        // 遍历JSON数组，填充到 int maze[][] 中
        for (int i = 0; i < height; ++i)
        {
            QJsonArray rowArray = mazeArray[i].toArray();
            for (int j = 0; j < width; ++j)
            {
                point current_pos = {i, j};
                QString cell = rowArray[j].toString();
                // 根据字符映射为整数
                if (cell == "#")
                    this->maze[i][j] = static_cast<int>(MAZE::WALL); // 墙
                else if (cell == " ")
                    this->maze[i][j] = static_cast<int>(MAZE::WAY);
                else if (cell == "S")
                {
                    this->maze[i][j] = static_cast<int>(MAZE::START); // 起点
                    this->start = point(i, j);
                }
                else if (cell == "E")
                {
                    this->maze[i][j] = static_cast<int>(MAZE::EXIT); // 终点
                    this->end = point(i, j);
                }
                else if (cell == "G")
                {
                    this->maze[i][j] = static_cast<int>(MAZE::SOURCE); // 金币
                    this->sourse_value[current_pos] = 50;
                    this->sourse.insert(current_pos);
                }
                else if (cell == "C")
                {
                    this->maze[i][j] = static_cast<int>(MAZE::WAY);
                }
                else if (cell == "B")
                {
                    this->maze[i][j] =
                        static_cast<int>(MAZE::BOSS);
                    this->boss_in_map = true;
                } // Boss
                else if (cell == "T")
                {
                    this->maze[i][j] = static_cast<int>(MAZE::TRAP); // 陷阱
                    this->traps[current_pos] = false;                // 存入陷阱地图，初值为false
                }
                else if (cell == "L")
                    this->maze[i][j] = static_cast<int>(MAZE::LOCKER); // 密码锁
                else
                    this->maze[i][j] = 0; // 默认为路
            }
        }
    }

    // 5. 处理 "B" (Boss HP) 数据
    if (jsonObj.contains("B") && jsonObj["B"].isArray())
    {
        QJsonArray bossArray = jsonObj["B"].toArray();

        // 清空旧数据，准备存入新数据
        this->bosshp.clear();

        for (const QJsonValue &hpValue : bossArray)
        {
            this->bosshp.push_back(hpValue.toInt());
        }
    }
    else
    {
        // QMessageBox::warning(this, "错误", "JSON文件中缺少'B'数组或格式不正确。");
        this->bosshp.clear();
        this->boss_in_map = true; // 读取未发现boss也标记，直接跳转到游戏结束界面
        // return;
    }
    if (jsonObj.contains("PlayerSkills") && jsonObj["PlayerSkills"].isArray())
    {
        QJsonArray skillsArray = jsonObj["PlayerSkills"].toArray();
        int id = 0;
        for (const QJsonValue &value : skillsArray)
        {
            if (value.isArray())
            {
                QJsonArray skillData = value.toArray();
                if (skillData.size() >= 2)
                {
                    Skill skill;
                    skill.id = id++;
                    skill.damage = skillData[0].toInt();
                    skill.cooldown = skillData[1].toInt();
                    Skills.push_back(skill);
                }
            }
        }
    }

    w=new MainWindow(this->dimension,model, this);
    if(dimension>0)
    {
        w->show();
    }
    this->hide();
    connect(w, &MainWindow::exit_mainwindow, this, &gamechoose::onExitButtonClicked);

    // if(this->dimension==0)
    // {
    //     boss* newboss=new boss(this->bosshp,this->Skills,20);
    //     newboss->show();
    //     this->hide();
    // }
    // else
    // {
    //     MainWindow*w=new MainWindow(this->mazesize,model, this);
    //     w->show();
    //     this->hide();
    // }

}

void gamechoose::onRandomButtonClicked()
{
    model = 2;
    bool ok;
    int mazeSize = QInputDialog::getInt(nullptr, "选择迷宫大小",
                                        "大小 (7-51, 建议为奇数):", 11, 7, 51, 1, &ok);

    if (ok) // 仅当用户点击“OK”时才创建和显示主窗口
    {
        w = new MainWindow(mazeSize, 2, this);
        connect(w, &MainWindow::exit_mainwindow, this, &gamechoose::onExitButtonClicked);
        w->show();
        this->hide();
    }
}

void gamechoose::onExitButtonClicked()
{
    w->deleteLater();
    w = nullptr;
    this->show();
}
