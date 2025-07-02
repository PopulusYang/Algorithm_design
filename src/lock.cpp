#include "heads/lock.h"
#include "ui_lock.h"

Lock::Lock(const QString &hash, const std::vector<gamemain::clue_content> &clues, QWidget *parent)
    : QDialog(parent), ui(new Ui::Lock)
{
    ui->setupUi(this);
    digits[0] = 0;
    digits[1] = 0;
    digits[2] = 0;
    update_display();

    ui->hash_label->setText("SHA256 Hash: " + hash);

    QString clueText = "已获得的线索:\n";
    for (const auto &clue : clues)
    {
        clueText += QString("密码第%1位 = %2\n").arg(clue.gen_order_index).arg(clue.password_dig_val);
    }
    ui->clue_label->setText(clueText);
}

Lock::~Lock()
{
    delete ui;
}

int Lock::getPassword() const
{
    return digits[0] * 100 + digits[1] * 10 + digits[2];
}

void Lock::update_display()
{
    ui->lcd_1->display(digits[0]);
    ui->lcd_2->display(digits[1]);
    ui->lcd_3->display(digits[2]);
}

void Lock::on_inc_1_clicked()
{
    digits[0] = (digits[0] + 1) % 10;
    update_display();
}

void Lock::on_dec_1_clicked()
{
    digits[0] = (digits[0] - 1 + 10) % 10;
    update_display();
}

void Lock::on_inc_2_clicked()
{
    digits[1] = (digits[1] + 1) % 10;
    update_display();
}

void Lock::on_dec_2_clicked()
{
    digits[1] = (digits[1] - 1 + 10) % 10;
    update_display();
}

void Lock::on_inc_3_clicked()
{
    digits[2] = (digits[2] + 1) % 10;
    update_display();
}

void Lock::on_dec_3_clicked()
{
    digits[2] = (digits[2] - 1 + 10) % 10;
    update_display();
}
