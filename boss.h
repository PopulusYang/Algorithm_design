#ifndef BOSS_H
#define BOSS_H

#include <QWidget>
#include"player.h"
#include<vector>
#include <limits>
#include<queue>
#include <QJsonDocument>  // 用于解析和生成JSON数据
#include <QJsonObject>    // 用于处理JSON对象
#include <QJsonArray>     // 用于处理JSON数组
#include <QFile>          // 用于读取文件
#include<math.h>
#include<QString>
namespace Ui {
class boss;
}


struct Skill
{
    int id;
    int damage;
    int cooldown;
};

struct Node
{
    int currturn;
    vector<int>bosshp;
    int currbossindex;
    vector<int>skicolldown;
    vector<pair<int,int>>path;
    int lowerbound;
    bool operator>(const Node&other)const
    {
        return lowerbound>other.lowerbound;
    }
};

class boss : public QWidget
{
    Q_OBJECT

public:
    explicit boss(QWidget *parent = nullptr);
    ~boss();
    void findBestStrate();
    int callowerBound(vector<int>&bosshp,vector<Skill>skills);
    void solveBossRush(vector<int>&bosshp,vector<Skill>&skills);
    vector<int> readAndwrite(QString filename);
    Player player;
private:
    Ui::boss *ui;
};

#endif // BOSS_H
