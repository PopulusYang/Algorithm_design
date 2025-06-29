#include "boss.h"
#include "ui_boss.h"

boss::boss(std::vector<Skill> Skills_param,
           std::vector<int> bosshp_param,
           QWidget *parent)
    : QWidget(parent),
    m_currentBossIndex(0),
    m_currentTurn(0),
    m_activeSkillId(-1)
{

    ui->setupUi(this);
    // 1. 通过构造函数参数初始化 Skills 和 skillid
    this->Skills = Skills_param;
    this->bosshp=bosshp_param;

    // 2. 调用函数来获得 bosshp

    this->skillid = solveBossRush(this->bosshp,this->Skills);

    // 3. 初始化其他内部状态变量
    m_currentBossHp = this->bosshp; // 使用获得的bosshp初始化当前血量
    m_skillCurrentCooldowns.resize(this->Skills.size(), 0);

    // 2. ★★★ 动态创建Boss血条控件
    for (size_t i = 0; i < this->bosshp.size(); ++i) {
        QProgressBar *bar = new QProgressBar(this);
        bar->setRange(0, this->bosshp[i]); // 设置进度条范围
        bar->setValue(this->bosshp[i]);   // 设置初始值
        bar->setFormat(QString("Boss %1: %v/%m").arg(i + 1)); // 设置显示格式
        bar->setAlignment(Qt::AlignCenter);

        // 使用样式表来设置颜色
        bar->setStyleSheet(
            "QProgressBar { border: 2px solid grey; border-radius: 5px; text-align: center; }"
            "QProgressBar::chunk { background-color: #2ECC71; }" // 绿色
            );

        ui->bossesLayout->addWidget(bar); // 添加到UI的布局中
        m_bossProgressBars.push_back(bar); // 保存指针以便后续更新
    }

    // 3. ★★★ 动态创建技能控件
    for (const auto& skill : this->Skills) {
        QWidget *skillContainer = new QWidget(this); // 每个技能的容器
        QVBoxLayout *layout = new QVBoxLayout(skillContainer);
        QLabel *label = new QLabel("Skill"+QString::number(skill.id), skillContainer);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        this->ui->skillsLayout->addWidget(skillContainer); // 添加到UI的布局

        // 保存指针
        m_skillWidgets.push_back(skillContainer);
        m_skillLabels.push_back(label);
    }

    // 4. 初始化定时器 (和之前一样)
    m_turnTimer = new QTimer(this);
    connect(m_turnTimer, &QTimer::timeout, this, &boss::nextTurn);

    // 初始更新一次UI状态
    updateUI();
}

boss::~boss()
{
    delete ui;
}

int boss::callowerBound(std::vector<int>&bosshp,std::vector<Skill>skills)//计算下界函数
{
    int totalremainhp;
    for(int hp:bosshp)
    {
        if(hp>0)totalremainhp+=hp;
    }
    if(totalremainhp==0)return 0;
    double avedamage=0;
    for(auto&skill:skills)
    {
        avedamage+=static_cast<double>(skill.damage)/(skill.cooldown+1);
    }
    if(avedamage==0)return std::numeric_limits<int>::max();
    return static_cast<int>(ceil(totalremainhp/avedamage));
}

std::vector<int> boss::solveBossRush(std::vector<int>&bosshp,std::vector<Skill>&skills)
{
    std::priority_queue<Node,std::vector<Node>,std::greater<Node>>pq;
    Node startNode;
    startNode.currturn=1;
    startNode.bosshp=bosshp;
    startNode.currbossindex=0;
    startNode.skicolldown.resize(skills.size(),0);
    startNode.lowerbound=callowerBound(startNode.bosshp,skills);

    pq.push(startNode);
    int minturns=std::numeric_limits<int>::max();
    std::vector<int>bestpath;
    while(!pq.empty())
    {
        Node currentNode=pq.top();
        pq.pop();
        if(currentNode.lowerbound>=minturns)continue;

        if(currentNode.currbossindex>=currentNode.bosshp.size())
        {
            if(currentNode.currturn-1<minturns)
            {
                minturns=currentNode.currturn-1;
                bestpath=currentNode.path;
            }
            continue;
        }
        bool skillUsed=false;
        for(int i=0;i<skills.size();i++)
        {
            if(currentNode.skicolldown[i]==0)
            {
                skillUsed=true;
                Node nextNode=currentNode;
                nextNode.currturn=currentNode.currturn+1;
                for(int j=0;j<nextNode.skicolldown.size();j++)
                {
                    if(nextNode.skicolldown[j]>0)nextNode.skicolldown[j]--;
                }
                nextNode.skicolldown[i]=skills[i].cooldown;
                nextNode.bosshp[nextNode.currbossindex]-=skills[i].damage;
                if(nextNode.bosshp[nextNode.currbossindex]<=0)
                {
                    nextNode.currbossindex++;
                }
                nextNode.path.push_back(skills[i].id);
                nextNode.lowerbound=(nextNode.currturn-1)+callowerBound(nextNode.bosshp,skills);
                if(nextNode.lowerbound<minturns)pq.push(nextNode);//分支限界
            }
        }
        if(!skillUsed)
        {
            Node nextNode=currentNode;
            nextNode.currturn=currentNode.currturn+1;
            for(int j=0;j<nextNode.skicolldown.size();j++)
            {
                if(nextNode.skicolldown[j]>0)nextNode.skicolldown[j]--;
            }
            nextNode.path.push_back(-1);
            nextNode.lowerbound=(nextNode.currturn-1)+callowerBound(nextNode.bosshp,skills);
            if(nextNode.lowerbound<minturns)
            {
                pq.push(nextNode);
            }
        }
    }

    return bestpath;
}


// std::vector<int> boss::readAndwrite()
// {
//     QString filename = "D:/codes/suanfa/Algorithm_design/data/boss_case_9.json";

//     // 调试输出当前路径
//     qDebug() << "当前工作目录:" << QDir::currentPath();
//     qDebug() << "尝试打开文件:" << filename;

//     if (!QFile::exists(filename)) {
//         qWarning() << "错误: 文件不存在" << QFileInfo(filename).absoluteFilePath();
//         return {};
//     }

//     QFile file(filename);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         qWarning() << "错误: 无法打开文件" << QFileInfo(filename).absoluteFilePath();
//         qWarning() << "错误信息:" << file.errorString();
//         return {};
//     }

//     QByteArray jsonData = file.readAll();
//     file.close();
//     std::vector<int>bossHP;
//     QJsonDocument doc = QJsonDocument::fromJson(jsonData);
//     QJsonObject jsonObject = doc.object();
//     if (jsonObject.contains("B") && jsonObject["B"].isArray())
//     {
//         QJsonArray bossArray = jsonObject["B"].toArray();
//         for (const QJsonValue &value : bossArray)
//         {
//             bossHP.push_back(value.toInt());
//         }
//     }
//     std::vector<Skill>skills;
//     if (jsonObject.contains("PlayerSkills") && jsonObject["PlayerSkills"].isArray())
//     {
//         QJsonArray skillsArray = jsonObject["PlayerSkills"].toArray();
//         int id = 0;
//         for (const QJsonValue &value : skillsArray)
//         {
//             if (value.isArray())
//             {
//                 QJsonArray skillData = value.toArray();
//                 if (skillData.size() >= 2)
//                 {
//                     Skill skill;
//                     skill.id = id++;
//                     skill.damage = skillData[0].toInt();
//                     skill.cooldown = skillData[1].toInt();
//                     skills.push_back(skill);
//                 }
//             }
//         }
//     }
//     std::vector<int>skillid=solveBossRush(bossHP,skills);
//     return skillid;
// }
void boss::showbossgame(std::vector<int>&bosshp,std::vector<Skill>Skills)
{
    std::vector<int>skillid=solveBossRush(bosshp,Skills);

}

void boss::updateUI()
{
    // 更新Boss血条
    for (size_t i = 0; i < m_bossProgressBars.size(); ++i) {
        m_bossProgressBars[i]->setValue(m_currentBossHp[i]);
    }

    // 更新技能显示
    for (size_t i = 0; i < this->Skills.size(); ++i) {
        QString styleSheet = "QWidget { background-color: #7F8C8D; border: 1px solid black; border-radius: 5px; }"; // 默认样式

        if (m_skillCurrentCooldowns[i] == 0) {
            // 技能就绪
            styleSheet = "QWidget { background-color: #3498DB; border: 1px solid black; border-radius: 5px; }";
            m_skillLabels[i]->setText("Skill"+QString::number(this->Skills[i].id) + "\nReady");
        } else {
            // 技能冷却中
            styleSheet = "QWidget { background-color: #95A5A6; border: 1px solid black; border-radius: 5px; }";
            m_skillLabels[i]->setText("Skill"+QString::number(this->Skills[i].id) + QString("\nCD: %1").arg(m_skillCurrentCooldowns[i]));
        }

        // 高亮当前回合使用的技能
        if (this->Skills[i].id == m_activeSkillId) {
            styleSheet += "QWidget { border: 4px solid #F1C40F; }"; // 亮黄色粗边框
        }

        m_skillWidgets[i]->setStyleSheet(styleSheet);
    }
}



void boss::nextTurn()
{
    // 逻辑中使用新的变量名
    if (m_currentBossIndex >= this->bosshp.size() || m_currentTurn >= this->skillid.size()) {
        m_turnTimer->stop();
        m_activeSkillId = -1;
        update();
        std::cout << "Simulation ended." << std::endl;
        return;
    }

    int skillIdToUse = this->skillid[m_currentTurn];
    m_activeSkillId = -1;

    int skillIndex = -1;
    for (size_t i = 0; i < this->Skills.size(); ++i) {
        if (this->Skills[i].id == skillIdToUse) {
            skillIndex = i;
            break;
        }
    }

    if (skillIndex != -1 && m_skillCurrentCooldowns[skillIndex] == 0) {
        const Skill& skill = this->Skills[skillIndex];
        m_activeSkillId = skill.id;

        m_currentBossHp[m_currentBossIndex] -= skill.damage;

        if (m_currentBossHp[m_currentBossIndex] < 0) {
            m_currentBossHp[m_currentBossIndex] = 0;
        }

        if (m_currentBossHp[m_currentBossIndex] == 0) {
            m_currentBossIndex++;
        }

        m_skillCurrentCooldowns[skillIndex] = skill.cooldown;
    } else {
        std::cout << "Turn " << m_currentTurn + 1 << ": Skill ID " << skillIdToUse << " is on cooldown or invalid." << std::endl;
    }

    for (int& cd : m_skillCurrentCooldowns) {
        if (cd > 0) {
            cd--;
        }
    }

    m_currentTurn++;
    updateUI();
}


// void boss::drawBossHealthBars(QPainter &painter)
// {
//     painter.save();
//     int barHeight = 30;
//     int barSpacing = 10;
//     int startY = 20;
//     int barWidth = width() - 40;

//     QFont font = painter.font();
//     font.setPixelSize(14);
//     painter.setFont(font);

//     // 使用 bosshp.size() 和 bosshp[i]
//     for (size_t i = 0; i < this->bosshp.size(); ++i) {
//         int y = startY + i * (barHeight + barSpacing);
//         QRectF totalRect(20, y, barWidth, barHeight);

//         painter.setBrush(QColor("#C0392B"));
//         painter.setPen(Qt::NoPen);
//         painter.drawRect(totalRect);

//         // 使用 bosshp[i] 作为最大血量
//         double hpRatio = static_cast<double>(m_currentBossHp[i]) / this->bosshp[i];
//         if (hpRatio < 0) hpRatio = 0;

//         painter.setBrush(QColor("#2ECC71"));
//         painter.drawRect(QRectF(20, y, barWidth * hpRatio, barHeight));

//         painter.setPen(Qt::black);
//         painter.setBrush(Qt::NoBrush);
//         painter.drawRect(totalRect);

//         QString text = QString("Boss %1: %2 / %3").arg(i + 1).arg(m_currentBossHp[i]).arg(this->bosshp[i]);
//         painter.setPen(Qt::white);
//         painter.drawText(totalRect, Qt::AlignCenter, text);
//     }
//     painter.restore();
// }

// void boss::drawSkillCooldownBars(QPainter &painter)
// {
//     painter.save();
//     int barHeight = 40;
//     int barWidth = 100;
//     int barSpacing = 15;
//     int startY = height() - barHeight - 20;
//     int startX = 20;

//     QFont font = painter.font();
//     font.setPixelSize(12);
//     painter.setFont(font);

//     // 使用 this->Skills
//     for (size_t i = 0; i < this->Skills.size(); ++i) {
//         int x = startX + i * (barWidth + barSpacing);
//         QRectF totalRect(x, startY, barWidth, barHeight);

//         if (this->Skills[i].id == m_activeSkillId) {
//             painter.setPen(QPen(QColor("#F1C40F"), 4));
//         } else {
//             painter.setPen(QPen(Qt::black, 1));
//         }

//         if (m_skillCurrentCooldowns[i] == 0) {
//             painter.setBrush(QColor("#3498DB"));
//             painter.drawRect(totalRect);
//             painter.setPen(Qt::white);
//             painter.drawText(totalRect, Qt::AlignCenter, "Skill"+QString::number(this->Skills[i].id) + "\nReady");
//         }
//         else {
//             painter.setBrush(QColor("#7F8C8D"));
//             painter.drawRect(totalRect);

//             double cdRatio = static_cast<double>(m_skillCurrentCooldowns[i]) / this->Skills[i].cooldown;

//             painter.setBrush(QColor("#BDC3C7"));
//             painter.setPen(Qt::NoPen);
//             painter.drawRect(QRectF(x, startY, barWidth * cdRatio, barHeight));

//             if (this->Skills[i].id == m_activeSkillId) painter.setPen(QPen(QColor("#F1C40F"), 4));
//             else painter.setPen(QPen(Qt::black, 1));
//             painter.setBrush(Qt::NoBrush);
//             painter.drawRect(totalRect);

//             painter.setPen(Qt::black);
//             QString text = "Skill"+QString::number(this->Skills[i].id) + QString("\nCD: %1").arg(m_skillCurrentCooldowns[i]);
//             painter.drawText(totalRect, Qt::AlignCenter, text);
//         }
//     }
//     painter.restore();
// }



