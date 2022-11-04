#ifndef TOOLSBARUI_H
#define TOOLSBARUI_H

#include <QToolBar>
#include <QWidget>
#include <QIcon>
#include <QAction>

class toolsbarUI : public QToolBar
{
    Q_OBJECT
public:
    explicit toolsbarUI(QWidget *parent = nullptr);

    //按键
    QAction *newTaskActionUI;//新建任务按钮
    QAction *exitActionUI;//退出按钮

private:
    //空白填充
    QWidget *spacer;

    QAction *continueTaskAction;//继续任务按钮
    QAction *pauseTaskAction;//暂停任务按钮
    QAction *cancelTaskAction;//取消任务按钮
    QAction *openFolderAction;//打开文件夹按钮
    QAction *lajiAction;//移至回收站按钮

signals:

};

#endif // TOOLSBARUI_H
