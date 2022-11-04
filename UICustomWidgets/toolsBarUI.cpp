#include "toolsBarUI.h"

toolsbarUI::toolsbarUI(QWidget *parent) : QToolBar(parent)
{
    //-------------定义按键---------------
    newTaskActionUI = new QAction(QIcon(":./images/new"), tr("&New Task"), this); // 新建按键
    newTaskActionUI->setShortcuts(QKeySequence::New);
    newTaskActionUI->setStatusTip(tr("New Task.")); // ctrl + N

    exitActionUI = new QAction(QIcon(":./images/exit"), tr("&Exit"), this); // 退出按键
    exitActionUI->setShortcuts(QKeySequence::Quit);
    exitActionUI->setStatusTip(tr("Exit."));

    continueTaskAction = new QAction(QIcon(":/images/start"), tr("&Continue"), this);
    continueTaskAction->setStatusTip(tr("Continue Task"));
    continueTaskAction->setEnabled(false);

    pauseTaskAction = new QAction(QIcon(":/images/pause"), tr("&Pause"), this);
    pauseTaskAction->setStatusTip(tr("Pause"));
    pauseTaskAction->setEnabled(false);

    cancelTaskAction = new QAction(QIcon(":/images/cancel"), tr("&Cancel Task"), this);
    cancelTaskAction->setShortcuts(QKeySequence::Cancel);
    cancelTaskAction->setStatusTip(tr("Cancel Task."));
    cancelTaskAction->setEnabled(false);

    openFolderAction = new QAction(QIcon(":/images/open"), tr("&Open Download Folder"), this);
    openFolderAction->setShortcuts(QKeySequence::Open);
    openFolderAction->setStatusTip(tr("Open Download Folder."));
    openFolderAction->setEnabled(false);

    lajiAction = new QAction(QIcon(":/images/delete"), tr("&Open Download Folder"), this);
    lajiAction->setStatusTip(tr("Remove."));
    lajiAction->setEnabled(false);

    //---------------布局-----------------
    this->setMovable(false);//设置工具栏不可拖动
    this->addAction(newTaskActionUI);// add button
    this->addAction(continueTaskAction);
    this->addAction(pauseTaskAction);
    this->addAction(cancelTaskAction);
    this->addAction(openFolderAction);

    spacer = new QWidget(this); // add spacer
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);//设置为Expanding属性
    this->addWidget(spacer);//工具栏添加空白条作填充，让最后一个按钮右顶格
    this->addAction(lajiAction);
    this->addSeparator(); // add separator
    this->addAction(exitActionUI);
}
