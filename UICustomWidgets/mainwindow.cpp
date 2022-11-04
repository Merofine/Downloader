#include "mainwindow.h"

//构造函数，主要用于构建GUI界面
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // ******** UI初始化 ******** //
    initWinUI(); // 主窗口UI
    createWinToolsUI();  // 工具栏UI
    createWinTabsUI(); // 标签管理UI
    createAddDialogUI(); // 下载对话框UI

    // ******** 信号&槽 ******** //
    connect(toolBarUI->exitActionUI, &QAction::triggered, this, &MainWindow::exit); // 按 退出，关闭软件
    connect(toolBarUI->newTaskActionUI, &QAction::triggered, this, &MainWindow::newTask); // 按 + ，弹出新建任务对话框
    connect(newTaskdiaUI, &NewTaskDialog::signalCreateNewTask, fTabUI, &filesTabUI::slotCreateNewTask); // 确定添加下载
}

MainWindow::~MainWindow()
{
    toolBarUI->deleteLater();
    newTaskdiaUI->deleteLater();
    fTabUI->deleteLater();
}

void MainWindow::initWinUI()
{
    // 主窗口初始化
    setWindowTitle(tr("CvteDownloader"));
    setWindowIcon(QIcon(tr(":/images/cvte.png")));
    resize(800, 600);
}

void MainWindow::createAddDialogUI()
{
    // 创建"新增下载任务对话框"
    newTaskdiaUI = new NewTaskDialog(this);
}

void MainWindow::createWinToolsUI()
{
    // 创建"工具栏"
    toolBarUI = new toolsbarUI(this);
    addToolBar(toolBarUI);
}

void MainWindow::createWinTabsUI()
{
    // 创建"标签管理" (正在下载、已完成、回收站)
    fTabUI = new filesTabUI(this);
    this->setCentralWidget(fTabUI);
}

//Open "新增下载任务对话框"
void MainWindow::newTask()
{
    newTaskdiaUI->open();
}

//退出
void MainWindow::exit()
{
    emit exitApp();//发送exitApp信号
}
