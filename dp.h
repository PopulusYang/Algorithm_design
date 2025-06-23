#ifndef DP_H
#define DP_H

#include <QWidget>
#include <cmath>

#include "gamemain.h"

namespace Ui {
class dp;
}

typedef struct point{
    int x;
    int y;
} point;

class dp : public QWidget, virtual public gamemain
{
    Q_OBJECT
    int **weight_table;

public:
    explicit dp(QWidget *parent = nullptr);
    ~dp();
    int weight(point dest, point current) const;

private:
    Ui::dp *ui;
};



#endif // DP_H
