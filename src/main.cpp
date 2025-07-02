#include "heads/gamechoose.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    gamechoose w;
    w.show();

    return a.exec();
}
