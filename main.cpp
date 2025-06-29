#include"gamechoose.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    gamechoose w;
    w.show();

    return a.exec();; // 如果用户取消，则程序正常退出
}
