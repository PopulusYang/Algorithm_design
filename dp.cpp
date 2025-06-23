#include "dp.h"
#include "ui_dp.h"

dp::dp(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::dp)
{
    ui->setupUi(this);
}

dp::~dp()
{
    delete ui;
}
