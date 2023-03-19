#include "basicFunc.h"

int nowBannersTotalheight = 0;

QString qNetwork_getHttpText(QString method, QString url, bool autoAlert)
{
	Network network;
	int code = network.sendHttpRequest(method, url);
	if (code == 200)
	{
		QString responseText = network.getHttpData();
		return responseText.toUtf8();
	}
	else
		return QString("network_error:%1").arg(code);
}

QByteArray qNetwork_getHttpData(QString method, QString url, bool autoAlert)
{
	Network network;
	int code = network.sendHttpRequest(method, url);
	if (code == 200)
		return network.getHttpData();
	else
		return NULL;
}

QString qNetwork_getHttpHeaderText( QString url, QString header, bool autoAlert)
{
	Network network;
	int code = network.sendHttpRequest("HEAD", url);
	if (code == 200)
		return network.getHttpHeader(header);
	else
		return QString("network_error:%1").arg(code);
}

QString qText_clearRFormat(QString text)
{
	text.replace("\r\n", "");//移除所有换行符
	text.replace("\n", "");
	text.replace("\t", "");//移除所有tab符
	return text;
}

int qText_indexOfEnd(QString text, QString indexText, int p)
{
	if (text.indexOf(indexText, p) != -1)
		return text.indexOf(indexText, p) + indexText.length();
	else
		return -1;
}

QString qText_Between(QString text, QString left, QString right, int from)
{
	const int from_left = text.indexOf(left, from) + left.length();
	const int n = text.indexOf(right, from_left) - from_left;
	return text.mid(from_left, n);
}

int qJson_findCurrentEnd(QString text, int nPos, QString sMark, QString eMark)
{
	int markw = 0;
	while (nPos < text.length() && markw >= 0)
	{
		if (text.at(nPos) == sMark)
			markw += 1;
		if (text.at(nPos) == eMark)
			markw -= 1;
		nPos += 1;
	}
	return nPos;
}

bool qFile_createDir(QString path)
{
	QDir dir;
	return dir.mkdir(path);
}

QString qConfig_readKey( QString key, QString configFileName)
{
	QFile configFile;
	configFile.setFileName(configFileName);
	if (configFile.size() > 1048576)
		return "ERROR_OOM";//如果目标配置文件大于1MB直接返回失败
	configFile.open(QIODevice::ReadOnly);
	QString configData = configFile.readAll();
	return qText_Between(configData, "\"", "\"", qText_indexOfEnd(configData, key));
}

bool qConfig_writeKey(QString configFileName, QString key, QString data)
{
	QFile configFile;
	configFile.setFileName(configFileName);
	if (configFile.size() > 1048576)
		return false;//如果目标配置文件大于1MB直接返回失败
	configFile.open(QIODevice::ReadWrite);
	QString configData = configFile.readAll();
	return true;
}

//void qMsgbox_info(void)
//{
//	msgBox* mb = new msgBox;
//	mb->setModal(true);
//	mb->type = 1;
//	mb->exec();
//}

Network::Network()
{
	sendBody = "";
	HRESULT isSuccessedCheck = CoInitialize(NULL);
	if (FAILED(isSuccessedCheck))
	{
		QMessageBox::critical(NULL, "RNC-意外错误",
			"一个意外的错误发生在basicFunc->::Network()->CoInitialize中, 无法初始化COM组件\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
	}
	pHttpWork.CreateInstance(__uuidof(WinHttp::WinHttpRequest));
	if (FAILED(isSuccessedCheck))
	{
		QMessageBox::critical(NULL, "RNC-意外错误",
			"一个意外的错误发生在basicFunc->::Network()->CreateInstance中, 无法初始化COM组件\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
	}
}

int Network::sendHttpRequest(QString method, QString url)
{
	HRESULT isSuccessedCheck;
	//DEBUG 此处的代理仅供调试使用
	//pHttpWork->SetProxy(2,L"127.0.0.1:8888");
	isSuccessedCheck = pHttpWork->Open(method.toStdWString().c_str(), url.toStdWString().c_str());
	if (FAILED(isSuccessedCheck))
	{
		return -3;
	}
	int nAddHeader = 0;
	while (headers[0].size() > nAddHeader)
	{
		pHttpWork->SetRequestHeader(headers[0].at(nAddHeader).toStdWString().c_str(), headers[1].at(nAddHeader).toStdWString().c_str());;
		nAddHeader += 1;
	}
	try
	{
		isSuccessedCheck = pHttpWork->Send(sendBody.toStdWString().c_str());
	}
	catch (...)
	{
		return -4;
	}
	return pHttpWork->Status;
}

QByteArray Network::getHttpData()
{
	_variant_t body = pHttpWork->GetResponseBody();
	ULONG dataLen = body.parray->rgsabound[0].cElements;
	QByteArray data((char*)body.parray->pvData, dataLen);
	return data;
}

QString Network::getHttpHeader(QString header)
{
	if (header == "")
	{
		_variant_t header = pHttpWork->GetAllResponseHeaders();
		ULONG dataLen = header.parray->rgsabound[0].cElements;
		QByteArray headerData((char*)header.parray->pvData, dataLen);
		return headerData;
	}
	else
	{
		return (LPCSTR)pHttpWork->GetResponseHeader(header.toStdWString().c_str());
	}
}

std::vector<MemberData> getMemberData(QString rawDataText)
{
	QString subString;
	UINT32 readPos = 0;
	QByteArray data = qText_Between(rawDataText, "<data>", "</data>", 0).toUtf8();//定位data键
	data = QByteArray::fromBase64(data);//base64解码
	const char antiReadKey = data.at(0);//读取混淆字节(首字节)
	data.remove(0, 1);//删除混淆字节
	int nConvPos = 0;
	while (nConvPos <= data.size() - 1)//反混淆循环
	{
		char convdByte = data.at(nConvPos) ^ antiReadKey;
		data.replace(nConvPos, 1, &convdByte, 1);
		nConvPos += 1;
	}
	QString dataText = qText_clearRFormat(data);

	std::vector<MemberData> result;
	MemberData nowReadingMemberData;
	readPos = qText_indexOfEnd(dataText, "<memberData>", 0);
	while (true)
	{
		subString = qText_Between(dataText, "[", "]", readPos);
		nowReadingMemberData.name = qText_Between(subString, "name=", ",", 0);
		nowReadingMemberData.weight = qText_Between(subString, "weight=", ",", 0).toUShort();
		result.push_back(nowReadingMemberData);
		readPos = qText_indexOfEnd(dataText, "]", readPos);
		if (dataText.indexOf("</memberData>", 0) <= readPos)
			break;
	}
	return result;
}



void saveMemberData(QFile* file, std::vector<MemberData> memberData, short lastRunningMode)
{
	QString writeText;
	writeText = QString("<manifest>\n\tfor=RNC_RandomNameCaller\n\tver=%1\n</manifest>\n")
				.arg(PROGRAMVERSION);
	QString rawDataText;
	rawDataText = QString("<info>\n\tlastRunningMode=%\n</info>\n<memberData>\n").arg(lastRunningMode);
	ushort nowNum = 0;
	while (nowNum <= memberData.size() - 1)
	{
		rawDataText.append(QString("\t[name=%1,weight=%2]\n").arg(memberData.at(nowNum).name).arg(memberData.at(nowNum).weight));
		nowNum += 1;
	}
	rawDataText.append("</memberData>");
	QByteArray data;
	const char antiReadCode = __rdtsc() % 0xFF;
	int nConvPos = 0;
	data = rawDataText.toUtf8();
	while (nConvPos <= data.size() - 1)
	{
		char convdByte = data.at(nConvPos) ^ antiReadCode;
		data.replace(nConvPos, 1, &convdByte, 1);
		nConvPos += 1;
	}
	data.insert(0, antiReadCode);
	data = data.toBase64();
	writeText += QString("<data>\n%1\n</data>").arg(data).toUtf8();
	file->write(writeText.toStdString().c_str());
}

bool saveMemberData_json(QString fileName, std::vector<MemberData> memberData)
{
	QFile memberDataFile;
	memberDataFile.setFileName(fileName);
	memberDataFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
	QJsonObject jsonobj_data;//data对象 (/)
	jsonobj_data.insert("LastRunningMode", "-1");
	QJsonArray jsonarr_memberData;//memberData主数组 (/MemberData)
	jsonarr_memberData.append("aa");
	jsonobj_data.insert("MemberData",jsonarr_memberData);

	// TODO 上次进行到此处，准备实现json保存

	memberDataFile.write(.toByteArray());
	memberDataFile.close();
	return true;
}

Banner::Banner(QWidget* parent)
	: QWidget(parent)
{
	ui_bn.setupUi(this);
}

void Banner::showBanner(QString styleType, QString text, int bannerWidth, int stayTime, bool enableAutoMovedown)
{
	ui_bn.mainTitle->setProperty("styleType", styleType);
	ui_bn.mainTitle->setStyleSheet(ui_bn.mainTitle->styleSheet());
	ui_bn.mainTitle->setText(text);
	ui_bn.mainTitle->setFixedWidth(bannerWidth);
	this->setGeometry(x(), -30, bannerWidth, 30);

	QPropertyAnimation* showAni = new QPropertyAnimation(this, "geometry");
	showAni->setStartValue(geometry());
	if (enableAutoMovedown)
	{
		showAni->setEndValue(QRect(x(), nowBannersTotalheight, bannerWidth, 30));
		autoMovedownEnabled = true;
		nowBannersTotalheight += height();
	}
	else
		showAni->setEndValue(QRect(x(), 0, bannerWidth, 30));
	showAni->setDuration(200);
	showAni->setEasingCurve(QEasingCurve::OutQuad);
	showAni->start();

	this->show();
	if (stayTime > 0)
	{
		connect(&hideTimer, &QTimer::timeout, this, &Banner::hideBanner);
		hideTimer.start(stayTime);
	}
	
}

void Banner::hideBanner(void)
{
	QPropertyAnimation* hideAni = new QPropertyAnimation(this, "geometry");
	hideAni->setStartValue(geometry());
	hideAni->setEndValue(QRect(x(), 0 - height(), width(), height()));
	hideAni->setDuration(200);
	hideAni->setEasingCurve(QEasingCurve::OutQuad);
	hideAni->start();
	if(autoMovedownEnabled)
		nowBannersTotalheight -= height();
	connect(hideAni, &QPropertyAnimation::finished, this, &Banner::closeBanner);
}

void Banner::closeBanner(void)
{
	this->close();
	this->deleteLater();
}