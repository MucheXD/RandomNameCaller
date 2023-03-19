#include "nameCaller.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    nameCaller mainWindow;
    mainWindow.show();
    return app.exec();
}
