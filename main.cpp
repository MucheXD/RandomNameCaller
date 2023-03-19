#include "nameCaller.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    nameCaller mainWindow;
    mainWindow.show();
    return app.exec();
}

/* 上次完成到创建调试按钮，尚未实现功能
*  基础文件拒读已完成，但未兼容旧版本配置文件
*  暂未实现配置文件自述校验 */