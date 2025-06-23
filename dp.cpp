#include "dp.h"
#include "./ui_dp.h"

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

int dp::weight(point dest, point current) const
{
    
    int lenghweight = (int)(sqrt((dest.x-current.x)*(dest.x-current.x)+(dest.y-current.y)*(dest.y-current.y))*100);
    int weight = lenghweight;
    weight += getMaze()[current.x][current.y] - 4 * 100;
    return weight;
}