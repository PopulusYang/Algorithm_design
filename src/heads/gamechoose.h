#ifndef GAMECHOOSE_H
#define GAMECHOOSE_H

#include <QWidget>
#include <QInputDialog>
#include <QJsonDocument>  // 用于解析和生成JSON数据
#include <QJsonObject>    // 用于处理JSON对象
#include <QJsonArray>     // 用于处理JSON数组
#include <QFile>          // 用于读取文件
#include <QDir>        // 用于目录操作
#include <QFileInfo>   // 用于获取文件信息
#include <QPainter>
#include <QPixmap>
#include<QLabel>
#include<QPushButton>
#include<QMessageBox>
#include<QFileDialog>
#include"gamemain.h"
#include"mainwindow.h"
#include"boss.h"//测试一下boss好不好使
namespace Ui {
class gamechoose;
}

class gamechoose : public QWidget,virtual public gamemain
{
    Q_OBJECT

public:
    explicit gamechoose(QWidget *parent = nullptr);
    ~gamechoose();
    void paintEvent(QPaintEvent *event) override;
    int model=-1;
private slots:
    void onFileButtonClicked();   // 用于响应 fileButton 点击事件的槽函数
    void onRandomButtonClicked(); // 用于响应 randomButton 点击事件的槽函数
    void onExitButtonClicked();

private:
    Ui::gamechoose *ui;
    MainWindow *w = nullptr;
};

#endif // GAMECHOOSE_H
