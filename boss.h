#ifndef BOSS_H
#define BOSS_H

#include <QWidget>

namespace Ui {
class boss;
}

class boss : public QWidget
{
    Q_OBJECT

public:
    explicit boss(QWidget *parent = nullptr);
    ~boss();

private:
    Ui::boss *ui;
};

#endif // BOSS_H
