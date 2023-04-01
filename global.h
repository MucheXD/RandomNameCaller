#pragma once
#include <QString>

const uint16_t PROGRAMMAGORVER = 0x0000;
const uint16_t PROGRAMMINORVER = 0x0000;
const uint16_t PROGRAMBUILDVER = 0x0018;
const QString PROGRAMTEXTID = "RNC_RandomNameCaller";

struct MemberData
{
    QString name;
    ushort weight;
};