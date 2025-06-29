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

class boss : public QWidget
{
    Q_OBJECT

public:
    explicit boss(QWidget *parent = nullptr);
    ~boss();
    void findBestStrate();
    int callowerBound(std::vector<int>&bosshp,std::vector<Skill>skills);
    std::vector<int> solveBossRush(std::vector<int>&bosshp,std::vector<Skill>&skills);
    std::vector<int> readAndwrite();
private:
    Ui::boss *ui;
};

#endif // BOSS_H
