﻿#include "basicFunc.h"

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

int8_t getMemberData(QString listFileName, std::vector<MemberData>* memberDataList)
{
	QFile listDataFile;
	listDataFile.setFileName(listFileName);
	if (!listDataFile.open(QIODevice::ReadOnly) || listDataFile.size() >= 0x100000)
		return -1;//无法打开或文件大小超过1MB的情况为异常
	QString mtdTextData = listDataFile.readAll();

	QString nSubBlockText;
	nSubBlockText = qText_Between(mtdTextData, "<manifest>", "</manifest>");
	QRegularExpression regExp;
	regExp.setPattern("(?<=\\[for=)\\S*(?=\\])");
	if (regExp.match(nSubBlockText).captured() != PROGRAMTEXTID)
		return -2;//配置与程序不符
	regExp.setPattern("(?<=\\[createrVersion=)\\S*(?=\\])");
	if (regExp.match(nSubBlockText).captured().toUShort() != PROGRAMBUILDVER)
		return -3;//配置与版本不符//TODO此处限制了唯一版本，应有版本兼容性
	nSubBlockText = qText_Between(mtdTextData, "<data>", "</data>");
	regExp.setPattern("(?<=\\[eptBlocks=)\\S*(?=\\])");
	const uint16_t eptBlocks = regExp.match(nSubBlockText).captured().toUShort();
	regExp.setPattern("(?<=\\[eptBytesPerBlock=)\\S*(?=\\])");
	const uint16_t bytesPerEptBlock = regExp.match(nSubBlockText).captured().toUShort();
	nSubBlockText = qText_Between(nSubBlockText, "[data=", "]");
	QByteArray mainData = QByteArray::fromBase64(nSubBlockText.toUtf8());
	nSubBlockText.clear();
	nSubBlockText.resize(0);

	const QByteArray eptKeyWord = mainData.left(eptBlocks);
	mainData.remove(0, eptBlocks);
	for (int nPos = 0; nPos < mainData.size(); nPos++)
		mainData[nPos] ^= eptKeyWord[nPos / bytesPerEptBlock];//再次异或比特，获得原数据
	for (int nCount=0; nCount < mainData.size() && mainData[mainData.size() - 1] != '}'; nCount++)
		mainData.resize(mainData.size() - 1);//去除对齐用数据，以'}'为界
	//读主数据体(JSON格式)
	QJsonDocument jsondoc;
	QJsonParseError jsonErrorChecker;
	jsondoc = QJsonDocument::fromJson(mainData, &jsonErrorChecker);
	if (jsonErrorChecker.error != 0)
		return 0xA0 + jsonErrorChecker.error;
	QJsonObject jsonobj_data = jsondoc.object();
	int lastRunningMode = jsonobj_data.value("LastRunningMode").toInt();//TODO 此处忽略了LastRunningMode的保存值，因为对应的功能未实现
	QJsonArray jsonarr_memberData = jsonobj_data.value("MemberData").toArray();//memberData主数组 (/MemberData)
	for (int nPos = 0; nPos < jsonarr_memberData.size(); nPos++)
	{
		QJsonObject jsonobj_indMember = jsonarr_memberData.at(nPos).toObject();
		MemberData nMemberData;
		nMemberData.name = jsonobj_indMember.value("n").toString();
		nMemberData.weight = jsonobj_indMember.value("w").toInt();
		memberDataList->push_back(nMemberData);
	}
	return 0;
}

bool saveMemberData(QString fileName, const std::vector<MemberData> *memberData)
{
	//构建主数据(使用JSON)
	QJsonDocument jsondoc;
	QJsonObject jsonobj_data;//data对象 (/)
	jsonobj_data.insert("LastRunningMode", "-1");
	QJsonArray jsonarr_memberData;//memberData主数组 (/MemberData)

	for (auto n_memberData : *memberData)
	{
		QJsonObject jsonobj_indMember;//新建单成员子对象 (/data/)
		jsonobj_indMember.insert("n", n_memberData.name.toUtf8().data());
		jsonobj_indMember.insert("w", n_memberData.weight);
		jsonarr_memberData.append(jsonobj_indMember);
	}
	jsonobj_data.insert("MemberData", jsonarr_memberData);
	jsondoc.setObject(jsonobj_data);
	//主数据拒读化
	QByteArray mainData = jsondoc.toJson(QJsonDocument::Compact);
	const uint16_t bytesPerEptBlock = 32;//每个使用不同keyByte的加密块的大小
	if (mainData.size() % bytesPerEptBlock != 0)
		mainData.resize((mainData.size() / bytesPerEptBlock + 1) * bytesPerEptBlock);//字节对齐
	const int eptBlocks = mainData.size() / bytesPerEptBlock;
	QByteArray eptKeyWord;
	for (int nPos = 0; nPos < eptBlocks; nPos++)
	{
		srand(__rdtsc() % 0xFFFFFFFF);
		eptKeyWord += rand();
	}
	for (int nPos = 0; nPos < mainData.size(); nPos++)
		mainData[nPos] ^= eptKeyWord[nPos / bytesPerEptBlock];
	mainData.insert(0, eptKeyWord);
	//构建附加内容(标准MTD)
	QString mtdStructText = QString("<manifest>\n\t[for=RNC_RandomNameCaller]\n\t[createrVersion=%1]\n</manifest>\n")
		.arg(PROGRAMBUILDVER);
	mtdStructText += QString("<data>\n\t[eptBlocks=%1]\n\t[eptBytesPerBlock=%2]\n\t[data=%3]\n</data>")
		.arg(eptBlocks).arg(bytesPerEptBlock).arg(mainData.toBase64());

	QFile listDataFile;
	listDataFile.setFileName(fileName);
	if (!listDataFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
		return false;
	listDataFile.write(mtdStructText.toUtf8());
	listDataFile.close();
	return true;
}

int8_t importMemberData(QString fileName, std::vector<MemberData>* memberData)
{
	QFileInfo fileInfo;
	fileInfo.setFile(fileName);
	QString fileSuffix = fileInfo.suffix();
	QFile file;
	file.setFileName(fileName);
	if (!file.open(QIODevice::ReadOnly) || file.size() >= 0x100000)
		return -1;
	if (fileSuffix == "txt" || fileSuffix == "csv");
	{
		while (!file.atEnd())
		{
			MemberData newMemberData;
			newMemberData.name = file.readLine().replace("\n", "").replace("\r", "");
			newMemberData.weight = 0;
			memberData->push_back(newMemberData);
		}
		return 0;
	}
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