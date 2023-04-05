#pragma once
#include <QCoreApplication>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QPropertyAnimation>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include "global.h"
#include "ui_bannerWidget.h"
#import "WinHttpCom.dll"

#pragma execution_character_set("utf-8")


QByteArray qNetwork_getHttpData(QString method, QString url, bool autoAlert);
QString qNetwork_getHttpText(QString method, QString url, bool autoAlert);
QString qNetwork_getHttpHeaderText(QString url, QString header, bool autoAlert);
QString qText_clearRFormat(QString text);
int qText_indexOfEnd(QString text, QString indexText, int p = 0);
QString qText_Between(QString text, QString left, QString right, int from = 0);
int qJson_findCurrentEnd(QString text, int nPos, QString sMark = "{", QString eMark = "}");
int qJson_findCurrentEnd(QString text, int nPos, QString sMark, QString eMark);
bool qFile_createDir(QString path);
QString qConfig_readKey(QString key, QString configFileName = QCoreApplication::applicationDirPath() + "/LUD.dcf");
bool qConfig_writeKey(QString configFile, QString key, QString data);
bool saveMemberData(QString fileName, const std::vector<MemberData> *memberData);
int8_t getMemberData(QString listFileName, std::vector<MemberData>* memberDataList);
int8_t importMemberData(QString fileName, std::vector<MemberData>* memberData);

class Network
{
public:
	WinHttp::IWinHttpRequestPtr pHttpWork;
	QString sendBody;
	QStringList headers[2];//0位存储标头 1位存储头内容
	Network();
	int sendHttpRequest(QString method, QString url);
	QByteArray getHttpData();//先发送请求
	QString getHttpHeader(QString header);//先发送请求
	/*结束代码
	* 1=成功
	* 0=未调用
	* -3=无法初始化连接
	* -4=无法获取数据
	*/
private:
};

class Banner : public QWidget
{
	Q_OBJECT

public:
	Banner(QWidget* parent = Q_NULLPTR);
	void showBanner(QString styleType, QString text, int bannerWidth, int stayTime, bool enableAutoMovedown = true);
	void hideBanner(void);

private:
	Ui::bannerWidgetClass ui_bn;
	QTimer hideTimer;
	bool autoMovedownEnabled = false;
	void closeBanner(void);
};


//class msgBox : public QDialog
//{
//	Q_OBJECT
//public:
//	msgBox(QDialog* parent = Q_NULLPTR);
//
//	int type = 0;
//
//private:
//	
//	int slideWindowCenterX = 0;
//	int slideWindowCenterY = 0;
//	void setWindowStyle(void);
//	void mouseMoveEvent(QMouseEvent* mouseEvent);
//	void mouseReleaseEvent(QMouseEvent* mouseEvent);
//	Ui::msgBoxW ui_mb;
//};
