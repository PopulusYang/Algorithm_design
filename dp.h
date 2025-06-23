#ifndef DP_H
#define DP_H

#include <QWidget>

namespace Ui {
class dp;
}

class dp : public QWidget
{
    Q_OBJECT

public:
    explicit dp(QWidget *parent = nullptr);
    ~dp();

private:
    Ui::dp *ui;
};

#endif // DP_H
