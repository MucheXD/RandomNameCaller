#pragma once

#include <Windows.h>
#include <vector>
#include <QFile>
#include <QDebug>
#include <QTranslator>
#include <QtWidgets/QWidget>
#include "basicFunc.h"
#include "ui_mainWindow.h"

class nameCaller : public QWidget
{
    Q_OBJECT

public:
    nameCaller(QWidget *parent = Q_NULLPTR);
    void reUi(void);
    void readMemberDataFromFile(QString fileName);
    void saveMemberDataToFile(QString fileName);
    void startChoosing(void);
    ushort makeChoice(void);
    void rollChoosing(ushort targetMember);
    void rollChoosingAniD(void);
    void candidateItemFlashAni(ushort targetNo);
    ushort findHighestWeight(void);
    void redistributeWeight(ushort);
    void changeMode(void);
    void weightInit(bool);
    void closeEvent(QCloseEvent* event);

private:
    Ui::mainWindowClass ui_mw;
    int nowRunningMode = 0;
    int nowRollTimes = 0;
    ushort currentMember = 0;
    ushort candidate_slot = 0;
};

class NameCallerCore
{
public:
    void setMemberData(std::vector<MemberData> memberData);
    void setCallerMode(int mode);
    bool initMemberWeight(bool forceToZero = false);
private:
    std::vector<MemberData> members;//成员数据
    int callerMode = 0;
};