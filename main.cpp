#include "mainwindow.h"
#include"boss.h"
#include <QApplication>
#include <QInputDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    boss newboss;
    newboss.readAndwrite();
    bool ok;
    int mazeSize = QInputDialog::getInt(nullptr, "选择迷宫大小",
                                        "大小 (7-51, 建议为奇数):", 11, 7, 51, 1, &ok);

    if (ok) // 仅当用户点击“OK”时才创建和显示主窗口
    {
        MainWindow w(mazeSize);
        w.show();
        return a.exec();
    }
    return 0; // 如果用户取消，则程序正常退出
}
