#pragma once

#include <Windows.h>
#include <vector>
#include <QFile>
#include <QDebug>
#include <QTranslator>
#include <QtWidgets/QWidget>
#include "basicFunc.h"
#include "ui_mainWindow.h"

struct MemberData
{
    QString name;
    UINT16 weight;
};


class nameCaller : public QWidget
{
    Q_OBJECT

public:
    nameCaller(QWidget *parent = Q_NULLPTR);
    void reUi(void);
    void startChoosing(void);
    UINT16 makeChoice(void);
    UINT16 findHighestWeight(void);
    void redistributeWeight(UINT16);
    void readMemberData(void);
    void changeMode(void);
    void weightInit(bool);

private:
    Ui::mainWindowClass ui_mw;
};
