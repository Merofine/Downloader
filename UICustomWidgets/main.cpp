#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // 点击指定按钮退出
    QObject::connect(&w, &MainWindow::exitApp, &a, &QApplication::quit);
    w.show();
    return a.exec();
}
