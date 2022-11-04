#ifndef FILESTABUI_H
#define FILESTABUI_H

#include <QWidget>
#include <QHBoxLayout>
#include <QDir>
#include "uiitemwidget.h"

struct tabList
{
    QString tabName;
    QString iconPath;
    QListWidget cList; // List
    QList<QListWidgetItem*> elementPlace; // List中的Item
    QList<uiItemWidget*> elementContent; // Item存放的Widget
};

class filesTabUI : public QWidget
{
    Q_OBJECT
public:
    explicit filesTabUI(QWidget *parent = nullptr);
    ~filesTabUI(void);

private:
    QTabWidget *tabWidget; // Tab页面
    QHBoxLayout  *hBoxLayout;

    struct tabList downloadTab;
    struct tabList finishedTab;
    struct tabList deletedTab;

    void initUI(void);

public slots:
    // 在“下载标签页”中添加新任务
    void slotCreateNewTask(struct newTaskMessage);

    // 从Tab中完全移除Item
    void deleteCompletely(void);

    // 下载文件->回收站
    void deleteEasyly(void);

    // 回收站->下载文件
    void returnTask(void);

    // 下载文件->已完成
    void finishedTask(void);

signals:

};

#endif // FILESTABUI_H
