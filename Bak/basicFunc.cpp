#include "basicFunc.h"

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
	sendBody = NULL;
	HRESULT isSuccessedCheck = CoInitialize(NULL);
	if (FAILED(isSuccessedCheck))
	{
		QMessageBox::critical(NULL, "LUD-意外错误",
			"一个意外的错误发生在basicFunc->::Network()->CoInitialize中, 无法初始化COM组件\n建议您重启程序, 如果仍无法解决, 请反馈给开发者");
	}
	pHttpWork.CreateInstance(__uuidof(WinHttp::WinHttpRequest));
	if (FAILED(isSuccessedCheck))
	{
		QMessageBox::critical(NULL, "LUD-意外错误",
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