#ifndef BOSS_H
#define BOSS_H

#include <QWidget>
#include <vector>
#include <QLabel>
#include <QProgressBar>
#include<queue>
#include"gamemain.h"


// 向前声明 UI 类，注意这里也是小写
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
// 类名改为小写
class boss : public QWidget
{
    Q_OBJECT

public:
    // 构造函数使用新的类名
    explicit boss(std::vector<int> initialBosshp,std::vector<Skill>initialSkills, QWidget *parent = nullptr);
    ~boss();

    std::vector<int> solveBossRush(std::vector<int>&bosshp,std::vector<Skill>&skills);
    int callowerBound(std::vector<int>&bosshp,std::vector<Skill>skills);
private slots:
    void onNextTurnClicked();

    void on_returnbotton_clicked();

private:
    void setupUiElements();
    void updateUI();
    void cleanupUiElements();

    Ui::boss *ui; // 指向小写 'b' 的 ui 类

    // --- 核心数据 (已按您的要求命名) ---
    std::vector<int> bosshp;
    std::vector<Skill> Skills;
    std::vector<int> skillid;

    // --- 内部战斗状态变量 ---
    std::vector<int> m_currentBossHp;
    int m_currentTurn;
    int m_currentTargetBossIndex;
    std::vector<int> m_currentSkillCooldowns;

    // // --- 动态创建的UI控件指针 ---
    std::vector<QLabel*> m_bossLabels;
    std::vector<QProgressBar*> m_bossHpBars;
    std::vector<QLabel*> m_skillLabels;
    std::vector<QProgressBar*> m_skillCooldownBars;

signals:
    void exit_bossui();
};

#endif // BOSS_H
