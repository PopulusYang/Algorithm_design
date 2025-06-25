#pragma once
#include <QDialog>
#include <QPushButton>
#include "player.h"
#include "autocontrol.h"

class AutoControlPanel : public QDialog
{
    Q_OBJECT
public:
    AutoControlPanel(autocontroller *controller, QWidget *parent = nullptr);
    autocontroller *controller;
};