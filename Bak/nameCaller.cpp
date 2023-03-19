#include "nameCaller.h"
#pragma execution_character_set("utf-8")

std::vector<MemberData> memberList;//虽然理论可以无限，但是限制65535个
QTranslator translator;

nameCaller::nameCaller(QWidget *parent)
    : QWidget(parent)
{
    ui_mw.setupUi(this);

    //TODO 这里为多语言做准备 当前并没有实际用处
    translator.load(QCoreApplication::applicationDirPath() + "/zh_cn.qm");//默认中文
    qApp->installTranslator(&translator);

    nameCaller::reUi();

    //QFile styleSheetFile;
    //styleSheetFile.setFileName(QCoreApplication::applicationDirPath() + "/default_light.qss");
    //styleSheetFile.open(QIODevice::ReadOnly);
    //const QString styleSheetText = QLatin1String(styleSheetFile.readAll());
    //qApp->setStyleSheet(styleSheetText);


    readMemberData();

    weightInit(false);

    //DEBUG 效率测试
    /*
    UINT64 startTick = clock();
    for (int i = 0; i < 100000; i++)
    {
        startChoosing();
    }
    qDebug() << "速度测试完毕 100000次点名耗时 " << clock() - startTick << " Tick";
    */

    connect(ui_mw.comboBox_mode,static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &nameCaller::changeMode);
    connect(ui_mw.btn_startChoosing,&QPushButton::clicked,this,&nameCaller::startChoosing);
}
void nameCaller::reUi()
{
    ui_mw.btn_startChoosing->setProperty("styleType","default");
    ui_mw.btn_startChoosing->setText(tr("抽取"));

    ui_mw.comboBox_mode->addItem(tr("均匀模式"));
    ui_mw.comboBox_mode->addItem(tr("完全随机"));
    ui_mw.comboBox_mode->addItem(tr("避免重复"));
    
    //ui_mw.btn_startChoosing->setStyleSheet("font-family:Arial,黑体;font-size:24px");
    //ui_mw.text_resultName->setStyleSheet("font-family:Arial,黑体;font-size:96px;font:Bold");
}
void nameCaller::changeMode(void)
{
    weightInit(false);
}
void nameCaller::startChoosing()//开始抽取
{
    if (ui_mw.btn_startChoosing->text() == tr("下一轮"))
    {
        ui_mw.text_resultName->setText(tr("--"));
        ui_mw.btn_startChoosing->setText(tr("抽取"));
        weightInit(false);
        return;
    }
    const UINT16 resultMemberNo = makeChoice();
    if (resultMemberNo == 65535)
        return;
    ui_mw.text_resultName->setText(memberList.at(resultMemberNo).name);
    redistributeWeight(resultMemberNo);
}
UINT16 nameCaller::findHighestWeight()//求当前最高权重者 返回成员编号
{
    UINT16 nowNum = 0;//当前扫描成员
    UINT16 highestWeight = 0;//当前最高权重数
    UINT16 highestWeightNo = 0;//当前最高权重者
    while (nowNum <= memberList.size() - 1)
    {
        if (memberList.at(nowNum).weight > highestWeight)
        {
            highestWeightNo = nowNum;
            highestWeight = memberList.at(nowNum).weight;
        }
        nowNum += 1;
    }
    return highestWeightNo;
}
UINT16 nameCaller::makeChoice()//根据权重随机选择一个成员,返回该成员在memberList中的位置
{
    //原理:先将所有成员权重相加,在(1~总权重)之间取随机数,判断该随机数属于哪一成员的权重范围
    UINT16 nowNum = 0;//当前计算成员
    UINT32 nowWeight = 0;//当前搜寻权重范围
    UINT32 totalWeight = 0;//总权重
    UINT32 resultWeightNum = 0;//取出的随机数

    //DEBUG 打印权重表与权重流失验证
    qDebug() << "##### 当前权重表 #####";
    while (nowNum <= memberList.size() - 1)//计算总权重
    {
        qDebug() << nowNum << " | " << memberList.at(nowNum).weight;
        totalWeight += memberList.at(nowNum).weight;
        nowNum += 1;
    }
    if (totalWeight <= 0)
    {
        if (ui_mw.comboBox_mode->currentText() == tr("避免重复"))
        {
            QMessageBox::information(this, tr("已全部抽取"), tr("全部成员都已抽取") + "\n" + tr("单击[下一轮]开始下一轮抽取"));
            ui_mw.btn_startChoosing->setText(tr("下一轮"));
        }
        else
        {
            weightInit(true);
        }
        return 65535;
    }
    /*
    * if (totalWeight == 10000 * memberList.size())
        qDebug() << "经验证没有权重流失!";
    else
        qDebug() << "警告!权重流失 当前:" << totalWeight;
    */
    resultWeightNum = (__rdtsc() % totalWeight)+1;
    nowNum = 0;
    while (resultWeightNum > nowWeight)
    {
        nowWeight += memberList.at(nowNum).weight;
        nowNum += 1;
    }
    return nowNum - 1;//返回成员编号 因为memberList中成员从0开始编号,因此减去1
}
void nameCaller::redistributeWeight(UINT16 selectedMember)//根据抽取过的成员重新分配权重
{
    //完全随机:不分配权重,所有成员权重仍为1
    if (ui_mw.comboBox_mode->currentText() == tr("均匀模式"))//均匀模式:将抽取过的成员权重扣除,分配给其他成员
    {
        UINT16 deWeight = 0;//从刚抽取成员中扣除的权重
        deWeight = (memberList.at(selectedMember).weight) - 1;
        memberList.at(selectedMember).weight = 1;//将权重扣除到1
        //分发权重
        const UINT16 diWeight = deWeight / memberList.size();//平均分给每一个成员,计算每一个成员将分到的权重
        UINT16 nowNum = 0;
        while (nowNum <= memberList.size() - 1)
        {
            memberList.at(nowNum).weight += diWeight;
            deWeight -= diWeight;
            nowNum += 1;
        }
        //将没有分完的权重还回(防止总权重越来越少)
        memberList.at(selectedMember).weight += deWeight;
        return;
    }
    if (ui_mw.comboBox_mode->currentText() == tr("避免重复"))//避免重复:
    {
        memberList.at(selectedMember).weight = 0;
        return;
    }
}
void nameCaller::readMemberData(void)
{
    QFile memberDataFile;
    memberDataFile.setFileName(QCoreApplication::applicationDirPath()+"/list.dcf");
    memberDataFile.open(QIODevice::ReadOnly);
    if (memberDataFile.isReadable() == false)
    {
        QMessageBox::critical(this,tr("名单读取失败"),tr("无法打开指定名单: list.dcf")+"\n"+tr("请检查该文件是否已移动、删除或加权"));
        exit(-1);
    }
    QString memberDataText = memberDataFile.read(10240);
    memberDataText = qText_clearRFormat(memberDataText);
    memberDataFile.close();

    QString subString;
    UINT32 readPos = 0;
    MemberData nowReadingMemberData;
    readPos = qText_indexOfEnd(memberDataText,"<memberData>",0);
    memberList.clear();
    while (true)
    {
        subString = qText_Between(memberDataText,"[","]",readPos);
        nowReadingMemberData.name = qText_Between(subString, "name=", ",", 0);
        memberList.push_back(nowReadingMemberData);
        readPos = qText_indexOfEnd(memberDataText,"]",readPos);
        if (memberDataText.indexOf("</memberData>", 0) <= readPos)
            break;
    }
    return;
}
void nameCaller::weightInit(bool isAbnormal)//初始化权重 isAbnormal标明了此次重置是否是异常的
{
    UINT16 nowNum = 0;
    if (isAbnormal)
    {
        QMessageBox::critical(this,tr("错误"),tr("执行操作时发生未知错误")+"\n" + tr("我们已经尝试重置了权重表，如果该问题继续发生，请求助程序管理员。"));
    }
    if (ui_mw.comboBox_mode->currentText()==tr("完全随机"))//完全随机:设置所有成员权重均为1
    {
        while (nowNum <= memberList.size() - 1)
        {
            memberList.at(nowNum).weight = 1;
            nowNum += 1;
        }
        return;
    }
    if (ui_mw.comboBox_mode->currentText() == tr("均匀模式"))//均匀模式:初始每位成员得到10000权重
    {
        while (nowNum <= memberList.size() - 1)
        {
            memberList.at(nowNum).weight = 10000;
            nowNum += 1;
        }
        return;
    }
    if (ui_mw.comboBox_mode->currentText() == tr("避免重复"))//避免重复:初始每位成员得到1权重
    {
        while (nowNum <= memberList.size() - 1)
        {
            memberList.at(nowNum).weight = 1;
            nowNum += 1;
        }
        if (isAbnormal = false)
            ui_mw.btn_startChoosing->setText("抽取");
        return;
    }
    //完全随机与避免重复代码上一致,但为了后期维护故做区分
}