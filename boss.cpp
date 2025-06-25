#include "boss.h"
#include "ui_boss.h"

boss::boss(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::boss)
{
    ui->setupUi(this);
}

boss::~boss()
{
    delete ui;
}
