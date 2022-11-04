#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "newTaskDialogUI.h"
#include "filesTabUI.h"
#include "toolsBarUI.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);//构造函数
    ~MainWindow();//析构函数

private:

    //工具栏
    toolsbarUI *toolBarUI;

    //新建任务对话框
    NewTaskDialog *newTaskdiaUI;

    //选项卡 (正在下载、已完成、回收站)
    filesTabUI *fTabUI;

    void initWinUI();
    void createWinToolsUI();
    void createWinTabsUI();
    void createAddDialogUI();

private slots:
    void newTask();//新建任务
    void exit();//退出

signals:
    void exitApp();//退出信号
};

#endif // MAINWINDOW_H
