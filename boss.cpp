#include "boss.h"
#include "ui_boss.h" // 确保 ui 文件名是 boss.ui
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <algorithm>
#include"gamechoose.h"
// 构造函数实现
boss::boss(std::vector<int>initialBosshp, std::vector<Skill> initialSkills, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::boss), // 使用小写的 ui::boss
    m_currentTurn(0),
    m_currentTargetBossIndex(0)
{
    ui->setupUi(this);


    this->bosshp = initialBosshp;
    this->Skills = initialSkills;
    this->m_currentBossHp = this->bosshp;

    m_currentSkillCooldowns.resize(this->Skills.size(), 0);

    this->skillid = solveBossRush(this->bosshp, this->Skills);

    setupUiElements();

    connect(ui->nextTurnButton, &QPushButton::clicked, this, &boss::onNextTurnClicked);

    updateUI();
}

boss::~boss()
{
    cleanupUiElements();
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


// "下一回合" 按钮的点击事件处理
void boss::onNextTurnClicked()
{
    if (m_currentTurn >= this->skillid.size() || m_currentTargetBossIndex >= this->bosshp.size()) {
        ui->turnLabel->setEnabled(false);
        if (m_currentTargetBossIndex >= this->bosshp.size()) {
            QMessageBox::information(this, "胜利", "所有Boss已被击败！");
        } else {
            QMessageBox::information(this, "战斗结束", "所有指令已执行完毕！");
        }
        return;
    }

    int skillIdToUse = this->skillid[m_currentTurn];

    for (size_t i = 0; i < m_currentSkillCooldowns.size(); ++i) {
        if (m_currentSkillCooldowns[i] > 0) {
            m_currentSkillCooldowns[i]--;
        }
    }

    if (skillIdToUse != -1) {
        auto it = std::find_if(this->Skills.begin(), this->Skills.end(),
                               [skillIdToUse](const Skill& s){ return s.id == skillIdToUse; });

        if (it != this->Skills.end()) {
            size_t skillIndex = std::distance(this->Skills.begin(), it);
            const Skill& usedSkill = *it;

            if (m_currentSkillCooldowns[skillIndex] == 0) {
                m_currentBossHp[m_currentTargetBossIndex] -= usedSkill.damage;
                m_currentSkillCooldowns[skillIndex] = usedSkill.cooldown;

                if (m_currentBossHp[m_currentTargetBossIndex] <= 0) {
                    m_currentBossHp[m_currentTargetBossIndex] = 0;
                    m_currentTargetBossIndex++;
                }
            } else {
                qWarning() << "回合 " << m_currentTurn << ": 尝试使用冷却中的技能 " << skillIdToUse;
            }
        }
    }
    m_currentTurn++;
    updateUI();

    if (m_currentTargetBossIndex >= this->bosshp.size()) {
        ui->turnLabel->setEnabled(false);
        QMessageBox::information(this, "胜利", "所有Boss已被击败！");
    }
}

// 动态创建UI元素
void boss::setupUiElements()
{
    for (size_t i = 0; i < this->bosshp.size(); ++i) {
        QLabel* label = new QLabel(QString("Boss %1 (HP: %2)").arg(i + 1).arg(this->bosshp[i]), this);
        QProgressBar* bar = new QProgressBar(this);
        bar->setMinimumHeight(28);
        bar->setRange(0, this->bosshp[i]);
        bar->setValue(this->bosshp[i]);
        bar->setTextVisible(true);
        bar->setFormat(QString("%v / %m"));
        bar->setStyleSheet(
            "QProgressBar {"
            "    border: 1px solid grey;"
            "    border-radius: 5px;"
            "    background-color: #EFEFEF;"      // 已损失血量的颜色 (浅灰色背景)
            "    text-align: center;"
            "    color: black;"                  // 进度条上的文字颜色
            "}"
            "QProgressBar::chunk {"
            "    background-color: #E53935;"      // 剩余血量的颜色 (鲜艳的红色)
            "    border-radius: 4px;"
            "}"
            );
        ui->bossLayout->addWidget(label);
        ui->bossLayout->addWidget(bar);
        m_bossLabels.push_back(label);
        m_bossHpBars.push_back(bar);
    }

    for (const auto& skill : this->Skills) {
        QLabel* label = new QLabel(QString("技能 %1 (伤害: %2, 冷却: %3)").arg(skill.id).arg(skill.damage).arg(skill.cooldown), this);
        QProgressBar* bar = new QProgressBar(this);
        bar->setMinimumHeight(28);
        bar->setRange(0, std::max(1, skill.cooldown));
        bar->setValue(0);
        bar->setTextVisible(true);
        bar->setFormat("冷却中: %v回合");
        bar->setStyleSheet(
            "QProgressBar {"
            "    border: 1px solid grey;"
            "    border-radius: 5px;"
            "    background-color: #D3D3D3;"      // 已冷却回合: 灰色
            "    text-align: center;"
            "    color: black;"
            "}"
            "QProgressBar::chunk {"
            "    background-color: #87CEEB;"      // 待冷却回合: 浅蓝色 (SkyBlue)
            "    border-radius: 4px;"
            "}"
            );
        ui->skillsLayout->addWidget(label);
        ui->skillsLayout->addWidget(bar);
        m_skillLabels.push_back(label);
        m_skillCooldownBars.push_back(bar);
    }
}

// 更新UI的函数
void boss::updateUI()
{
    QStringList usedSkillsHistory;
    // m_currentTurn 代表已完成的回合数，所以我们遍历 0 到 m_currentTurn - 1
    for (int i = 0; i < m_currentTurn; ++i) {
        int id = this->skillid[i];
        if (id == -1) {
            usedSkillsHistory.append("等待"); // 将 -1 表示为 "等待"
        } else {
            usedSkillsHistory.append(QString::number(id));
        }
    }
    QString historyString = usedSkillsHistory.join(" -> ");
    if (historyString.isEmpty()) {
        historyString = "无"; // 如果还没有使用技能，则显示 "无"
    }

    QString fullLabelText = QString("当前回合: %1 | 已用技能序列: %2")
                                .arg(m_currentTurn)
                                .arg(historyString);

    // 3. 更新 turnLabel
    ui->turnLabel->setText(fullLabelText);


    for (size_t i = 0; i < m_bossHpBars.size(); ++i) {
        m_bossHpBars[i]->setValue(m_currentBossHp[i]);
    }

    int skillIdThisTurn = -1;
    if (m_currentTurn < this->skillid.size()) {
        skillIdThisTurn = this->skillid[m_currentTurn];
    }

    for (size_t i = 0; i < this->Skills.size(); ++i) {
        const auto& skill = this->Skills[i];
        m_skillCooldownBars[i]->setValue(m_currentSkillCooldowns[i]);
        if (skill.cooldown == 0) {
            // 冷却为0的技能，文本是永久的
            m_skillCooldownBars[i]->setFormat("始终就绪");
        } else if (m_currentSkillCooldowns[i] == 0) {
            // 冷却>0的技能，冷却完毕后显示“已就绪”
            m_skillCooldownBars[i]->setFormat("已就绪");
        } else {
            // 正在冷却中
            m_skillCooldownBars[i]->setFormat(QString("冷却中: %1 回合").arg(m_currentSkillCooldowns[i]));
        }

        if (this->Skills[i].id == skillIdThisTurn) {
            m_skillLabels[i]->setStyleSheet("background-color: yellow; font-weight: bold;");
        } else {
            m_skillLabels[i]->setStyleSheet("");
        }
    }
}

void boss::cleanupUiElements()
{
    for(auto p : m_bossLabels) delete p;
    for(auto p : m_bossHpBars) delete p;
    for(auto p : m_skillLabels) delete p;
    for(auto p : m_skillCooldownBars) delete p;
    m_bossLabels.clear();
    m_bossHpBars.clear();
    m_skillLabels.clear();
    m_skillCooldownBars.clear();
}

void boss::on_returnbotton_clicked()
{
    gamechoose*newgame=new gamechoose();
    newgame->show();
    this->hide();
}

