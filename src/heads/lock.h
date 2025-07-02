#ifndef LOCK_H
#define LOCK_H

#include <QDialog>
#include "gamemain.h"
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Lock;
}
QT_END_NAMESPACE

class Lock : public QDialog
{
    Q_OBJECT

public:
    Lock(const QString &hash, const std::vector<gamemain::clue_content> &clues, QWidget *parent = nullptr);
    ~Lock();
    int getPassword() const;

private slots:
    void on_inc_1_clicked();
    void on_dec_1_clicked();
    void on_inc_2_clicked();
    void on_dec_2_clicked();
    void on_inc_3_clicked();
    void on_dec_3_clicked();

private:
    Ui::Lock *ui;
    int digits[3];
    void update_display();
};

#endif // LOCK_H