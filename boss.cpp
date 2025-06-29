#include "boss.h"
#include "ui_boss.h"

boss::boss(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::boss)
{
    ui->setupUi(this);
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


std::vector<int> boss::readAndwrite()
{
    QString filename = "D:/codes/suanfa/Algorithm_design/data/boss_case_9.json";

    // 调试输出当前路径
    qDebug() << "当前工作目录:" << QDir::currentPath();
    qDebug() << "尝试打开文件:" << filename;

    if (!QFile::exists(filename)) {
        qWarning() << "错误: 文件不存在" << QFileInfo(filename).absoluteFilePath();
        return {};
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "错误: 无法打开文件" << QFileInfo(filename).absoluteFilePath();
        qWarning() << "错误信息:" << file.errorString();
        return {};
    }

    QByteArray jsonData = file.readAll();
    file.close();
    std::vector<int>bossHP;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject jsonObject = doc.object();
    if (jsonObject.contains("B") && jsonObject["B"].isArray())
    {
        QJsonArray bossArray = jsonObject["B"].toArray();
        for (const QJsonValue &value : bossArray)
        {
            bossHP.push_back(value.toInt());
        }
    }
    std::vector<Skill>skills;
    if (jsonObject.contains("PlayerSkills") && jsonObject["PlayerSkills"].isArray())
    {
        QJsonArray skillsArray = jsonObject["PlayerSkills"].toArray();
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
                    skills.push_back(skill);
                }
            }
        }
    }

    std::vector<int>skillid=solveBossRush(bossHP,skills);

    return skillid;
}








