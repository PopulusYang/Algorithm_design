#ifndef BOSS_H
#define BOSS_H

#include <QWidget>
#include"gamemain.h"
#include<vector>
#include <limits>
#include<queue>
#include <QJsonDocument>  // 用于解析和生成JSON数据
#include <QJsonObject>    // 用于处理JSON对象
#include <QJsonArray>     // 用于处理JSON数组
#include <QFile>          // 用于读取文件
#include <QDir>        // 用于目录操作
#include <QFileInfo>   // 用于获取文件信息
#include<math.h>
#include<QString>
#include<qdebug.h>
#include <QWidget>
#include <vector>
#include <string>
#include <map>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QColor>
#include <QFont>
#include <iostream>
#include<QProgressBar>
#include<QLabel>
namespace Ui {
class boss;
}

struct Node
{
    int currturn;
    std::vector<int>bosshp;
    int currbossindex;
    std::vector<int>skicolldown;
    std::vector<int>path;
    int lowerbound;
    bool operator>(const Node&other)const
    {
        return lowerbound>other.lowerbound;
    }
};
class QTimer;
class boss : public QWidget
{
    Q_OBJECT
public:
    explicit boss(std::vector<Skill> Skills_param,
                  std::vector<int> bosshp_param,
                  QWidget *parent=NULL);

    // 析构函数名已修改
    ~boss();

    // 提供一个公共方法来开始模拟
    void startSimulation(int turnIntervalMsec = 1000);
    void findBestStrate();
    int callowerBound(std::vector<int>&bosshp,std::vector<Skill>skills);
    std::vector<int> solveBossRush(std::vector<int>&bosshp,std::vector<Skill>&skills);
    std::vector<int> readAndwrite();
    void showbossgame(std::vector<int>&bosshp,std::vector<Skill>Skills);


private:
    Ui::boss *ui;
    // 内部逻辑函数
    void updateUI();
    // void drawBossHealthBars(QPainter &painter);
    // void drawSkillCooldownBars(QPainter &painter);

    // --- 游戏状态数据 ---

    std::vector<int> bosshp;  // 存储Boss的初始/最大血量
    std::vector<Skill> Skills;
    std::vector<int> skillid;

    // 内部状态变量
    std::vector<int> m_currentBossHp; // 仍需要一个变量来追踪Boss当前血量的动态变化
    int m_currentBossIndex;
    std::map<int, Skill> m_skillMap;
    std::vector<int> m_skillCurrentCooldowns;
    int m_currentTurn;
    int m_activeSkillId;
    QTimer *m_turnTimer;
    std::vector<QProgressBar*> m_bossProgressBars;
    std::vector<QWidget*> m_skillWidgets; // 每个技能对应一个容器控件
    std::vector<QLabel*> m_skillLabels;
// protected:
//     // 覆盖QWidget的paintEvent函数，所有的绘制都在这里进行
//     void paintEvent(QPaintEvent *event) override;

private slots:
    // 一个槽函数，用于处理每个回合的逻辑
    void nextTurn();

};

#endif // BOSS_H
