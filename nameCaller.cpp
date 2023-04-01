#include "nameCaller.h"
#pragma execution_character_set("utf-8")

std::vector<MemberData> memberData;//虽然理论可以无限，但是限制65535个
QTranslator translator;

nameCaller::nameCaller(QWidget *parent)
    : QWidget(parent)
{
    ui_mw.setupUi(this);

    //TODO 这里为多语言做准备 当前并没有实际用处
    translator.load(QCoreApplication::applicationDirPath() + "/zh_cn.qm");//默认中文
    qApp->installTranslator(&translator);

    reUi();

    //QFile styleSheetFile;
    //styleSheetFile.setFileName(QCoreApplication::applicationDirPath() + "/default_light.qss");
    //styleSheetFile.open(QIODevice::ReadOnly);
    //const QString styleSheetText = QLatin1String(styleSheetFile.readAll());
    //qApp->setStyleSheet(styleSheetText);

    //TODO 此处固定了成员表
    readMemberDataFromFile(QCoreApplication::applicationDirPath() + "/list.dcf");

    //DEBUG JSON保存测试代码
    saveMemberData("list_json.mtd", memberData);

    getMemberData_new("list_json.mtd", &memberData);


    //weightInit(false);

    //DEBUG 效率测试
    /*
    UINT64 startTick = clock();
    for (int i = 0; i < 100000; i++)
    {
        startChoosing();
    }
    qDebug() << "速度测试完毕 100000次点名耗时 " << clock() - startTick << " Tick";
    */

    connect(ui_mw.btn_startChoosing,&QPushButton::clicked,this,&nameCaller::startChoosing);
    connect(ui_mw.btn_mode_pseudoRandom, &QPushButton::clicked, this, &nameCaller::changeMode);
    connect(ui_mw.btn_mode_completelyRandom, &QPushButton::clicked, this, &nameCaller::changeMode);
    connect(ui_mw.btn_mode_avoidRepeat, &QPushButton::clicked, this, &nameCaller::changeMode);
}

void nameCaller::reUi()
{
    ui_mw.btn_startChoosing->setProperty("styleType","default");
    Banner* banner = new Banner(this);
    banner->showBanner("notice", "这是一个开发中的程序, 许多功能尚未完善并可能存在问题, 不过我们会努力改进哒w", this->width(), 10000);

    ui_mw.widget_candidateArea->setVisible(false);

    //banner->setParent(this);
    //banner.show();
    
    //ui_mw.btn_startChoosing->setStyleSheet("font-family:Arial,黑体;font-size:24px");
    //ui_mw.text_resultName->setStyleSheet("font-family:Arial,黑体;font-size:96px;font:Bold");
}

void nameCaller::readMemberDataFromFile(QString fileName)
{
    QFile memberDataFile;
    memberDataFile.setFileName(fileName);
    memberDataFile.open(QIODevice::ReadOnly);
    if (memberDataFile.isReadable() == false)
    {
        QMessageBox::critical(this, tr("名单读取失败"), tr("无法打开指定名单: list.dcf") + "\n" + tr("请检查该文件是否已移动、删除或无法访问"));
        exit(-1);
    }
    QString memberDataText = memberDataFile.read(10240);
    memberDataText = qText_clearRFormat(memberDataText);
    memberDataFile.close();
    memberData = getMemberData(memberDataText);
    return;
}

void nameCaller::saveMemberDataToFile(QString fileName)
{
    saveMemberData("list.mtd", memberData);
}

void nameCaller::changeMode(void)
{
    QPushButton* senderCopy_btn = qobject_cast<QPushButton*>(sender());
    nowRunningMode = senderCopy_btn->property("modeID").toInt();
    weightInit(false);
}

void nameCaller::startChoosing()//开始抽取
{
    //int a = 0;
    //QString rate_A;
    //while (a < 1000)
    //{
    //    int nowNum = 0;
    //    int totalWeight = 0;
    //    while (nowNum <= memberData.size() - 1)//计算总权重
    //    {
    //        totalWeight += memberData.at(nowNum).weight;
    //        nowNum += 1;
    //    }
    //    rate_A.append(QString::number(totalWeight) + "\n");
    //    const ushort resultMemberNo = makeChoice();
    //    redistributeWeight(resultMemberNo);
    //    a += 1;
    //    Sleep(resultMemberNo * 2);
    //}


    const ushort resultMemberNo = makeChoice();
    if (resultMemberNo == 65535)
        return;

    //ui_mw.text_resultName->setText(memberData.at(resultMemberNo).name);
    rollChoosing(resultMemberNo);

    redistributeWeight(resultMemberNo);
}

void nameCaller::rollChoosing(ushort targetMember)
{
    nowRollTimes = 0;
    currentMember = targetMember;
    candidate_slot = __rdtsc() % 3 + 1;
	ui_mw.widget_candidateArea->setVisible(true);
	ui_mw.text_choosingInfo->setVisible(false);
    ui_mw.btn_startChoosing->setEnabled(false);
    ui_mw.widget_modeBox->setEnabled(false);
    ui_mw.candidateMember_1->setProperty("styleType", -1);
    ui_mw.candidateMember_2->setProperty("styleType", -1);
    ui_mw.candidateMember_3->setProperty("styleType", -1);
    ui_mw.candidateMember_1->setText("待选成员");
    ui_mw.candidateMember_2->setText("待选成员");
    ui_mw.candidateMember_3->setText("待选成员");
    ui_mw.widget_candidateArea->setStyleSheet(ui_mw.widget_candidateArea->styleSheet());
    QTimer* nextCallTimer = new QTimer;
	nextCallTimer->start(50);
	connect(nextCallTimer, &QTimer::timeout, this, &nameCaller::rollChoosingAniD);
}

void nameCaller::rollChoosingAniD()
{
    nowRollTimes += 1;
    if (nowRollTimes < 40)
    {
        ui_mw.text_resultName->setText(memberData.at(__rdtsc() % memberData.size()).name);
        QString nowSlotText = "";

        if (nowRollTimes % 10 == 0)
            if (nowRollTimes / 10 != candidate_slot)
                nowSlotText = ui_mw.text_resultName->text();
            else
            {
                nowSlotText = memberData.at(currentMember).name;
                ui_mw.text_resultName->setText(memberData.at(currentMember).name);
            }  
        if (nowRollTimes == 10)
            ui_mw.candidateMember_1->setText(nowSlotText);
        if (nowRollTimes == 20)
            ui_mw.candidateMember_2->setText(nowSlotText);
        if (nowRollTimes == 30)
            ui_mw.candidateMember_3->setText(nowSlotText);
        candidateItemFlashAni(nowRollTimes / 10);
        return;
    }
    if (nowRollTimes == 40)
    {
        QTimer* senderCopy_timer = qobject_cast<QTimer*>(sender());
        ui_mw.text_resultName->setText("最终抉择!");
        senderCopy_timer->start(1000);
        return;
    }
    if (nowRollTimes == 41)
    {
        QTimer* senderCopy_timer = qobject_cast<QTimer*>(sender());
        senderCopy_timer->start(1000);
        if (candidate_slot == 1)
            ui_mw.candidateMember_1->setProperty("styleType", 1);
        if (candidate_slot == 2)
            ui_mw.candidateMember_2->setProperty("styleType", 1);
        if (candidate_slot == 3)
            ui_mw.candidateMember_3->setProperty("styleType", 1);
        ui_mw.widget_candidateArea->setStyleSheet(ui_mw.widget_candidateArea->styleSheet());
        ui_mw.text_resultName->setText(memberData.at(currentMember).name);
        return;
    }
    if (nowRollTimes >= 42)
    {
        QTimer* senderCopy_timer = qobject_cast<QTimer*>(sender());
        senderCopy_timer->stop();
        senderCopy_timer->deleteLater();
        ui_mw.widget_candidateArea->setVisible(false);
        ui_mw.text_choosingInfo->setVisible(true);
        ui_mw.btn_startChoosing->setEnabled(true);
        ui_mw.widget_modeBox->setEnabled(true);
        return;
    }
}

void nameCaller::candidateItemFlashAni(ushort targetNo)
{
    if (targetNo == 1)
        ui_mw.candidateMember_1->setProperty("styleType", 0);
    if (targetNo == 2)
        ui_mw.candidateMember_2->setProperty("styleType", 0);
    if (targetNo == 3)
        ui_mw.candidateMember_3->setProperty("styleType", 0);
    ui_mw.widget_candidateArea->setStyleSheet(ui_mw.widget_candidateArea->styleSheet());
}

ushort nameCaller::findHighestWeight()//求当前最高权重者 返回成员编号
{
    UINT16 nowNum = 0;//当前扫描成员
    UINT16 highestWeight = 0;//当前最高权重数
    UINT16 highestWeightNo = 0;//当前最高权重者
    while (nowNum <= memberData.size() - 1)
    {
        if (memberData.at(nowNum).weight > highestWeight)
        {
            highestWeightNo = nowNum;
            highestWeight = memberData.at(nowNum).weight;
        }
        nowNum += 1;
    }
    return highestWeightNo;
}

ushort nameCaller::makeChoice()//根据权重随机选择一个成员,返回该成员在memberList中的位置
{
    //原理:先将所有成员权重相加,在(1~总权重)之间取随机数,判断该随机数属于哪一成员的权重范围
    UINT16 nowNum = 0;//当前计算成员
    UINT32 nowWeight = 0;//当前搜寻权重范围
    UINT32 totalWeight = 0;//总权重
    UINT32 resultWeightNum = 0;//取出的随机数

    ////DEBUG 打印权重表与权重流失验证
    //qDebug() << "##### 当前权重表 #####";
    while (nowNum <= memberData.size() - 1)//计算总权重
    {
        //qDebug() << nowNum << " | " << memberData.at(nowNum).weight;
        totalWeight += memberData.at(nowNum).weight;
        nowNum += 1;
    }
    if (totalWeight <= 0)
    {
        if (nowRunningMode == 2)
        {
            //QMessageBox::information(this, tr("已全部抽取"), tr("全部成员都已抽取") + "\n" + tr("继续抽取将开始下一轮"));
            Banner* banner = new Banner(this);
            banner->showBanner("notice", "所有成员都已经抽取过啦! 继续抽取将开始下一轮...", this->width(), 5000);
            ui_mw.text_resultName->setText(tr("等待抽取"));
            weightInit(false);
        }
        else
        {
            weightInit(true);
        }
        return 65535;
    }
    resultWeightNum = (__rdtsc() % totalWeight) + 1;
    nowNum = 0;
    while (resultWeightNum > nowWeight)
    {
        nowWeight += memberData.at(nowNum).weight;
        nowNum += 1;
    }
    return nowNum - 1;//返回成员编号 因为memberList中成员从0开始编号,因此减去1
}

void nameCaller::redistributeWeight(ushort selectedMember)//根据抽取过的成员重新分配权重
{
    //完全随机:不分配权重,所有成员权重仍为1
    if (nowRunningMode == 0)//均匀模式:将抽取过的成员权重扣除,分配给其他成员
    {
        memberData.at(selectedMember).weight = 0;//将被选中者权重扣除
        //分发权重
        ushort nowNum = 0;
        while (nowNum <= memberData.size() - 1)//增加其余未选中者权重1
        {
            memberData.at(nowNum).weight += 1;
            nowNum += 1;
        }
        return;
    }
    if (nowRunningMode == 2)//避免重复:
    {
        memberData.at(selectedMember).weight = 0;
        return;
    }
}

void nameCaller::weightInit(bool isAbnormal)//初始化权重 isAbnormal标明了此次重置是否是异常的
{
    ushort nowNum = 0;
    if (isAbnormal)
    {
        Banner* banner = new Banner(this);
        banner->showBanner("critical", "出...出错了Σ(っ °Д °;)っ, 已经为您尝试重置权重表", this->width(), 5000);
        //QMessageBox::critical(this,tr("错误"),tr("执行操作时发生未知错误")+"\n" + tr("我们已经尝试重置了权重表，如果该问题继续发生，请求助程序管理员。"));
    }
    //为每个成员分配权重
    while (nowNum <= memberData.size() - 1)
    {
        if (nowRunningMode == 0)
            memberData.at(nowNum).weight = memberData.size() / 2 + 1;
        else
            memberData.at(nowNum).weight = 1;
        nowNum += 1;
    }
    if (isAbnormal || nowRunningMode == 0)
    {
        Banner* banner = new Banner(this);
        banner->showBanner("warning", "权重表已经重置", this->width(), 1500, false);
    }
}

void nameCaller::closeEvent(QCloseEvent* event)
{
    saveMemberDataToFile(QCoreApplication::applicationDirPath() + "/list.dcf");
}