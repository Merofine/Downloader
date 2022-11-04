#include "filesTabUI.h"

filesTabUI::filesTabUI(QWidget *parent) : QWidget(parent)
{
    initUI();
}

filesTabUI::~filesTabUI(void)
{
    tabWidget->deleteLater();
    hBoxLayout->deleteLater();
}

void filesTabUI::initUI(void)
{
    // tab的初始化以及添加
    tabWidget = new QTabWidget;

    downloadTab.tabName = tr("正在下载");
    downloadTab.iconPath = tr(":/images/tabDown");
    downloadTab.cList.setSpacing(4);
    tabWidget->addTab(&(downloadTab.cList),
                      QIcon(downloadTab.iconPath),
                      downloadTab.tabName
                      );

    finishedTab.tabName = tr("已完成");
    finishedTab.iconPath = tr(":/images/tabF");
    finishedTab.cList.setSpacing(4);
    tabWidget->addTab(&(finishedTab.cList),
                      QIcon(finishedTab.iconPath),
                      finishedTab.tabName
                      );

    deletedTab.tabName = tr("回收站");
    deletedTab.iconPath = tr(":/images/tabL");
    deletedTab.cList.setSpacing(4);
    tabWidget->addTab(&(deletedTab.cList),
                      QIcon(deletedTab.iconPath),
                      deletedTab.tabName
                      );

    hBoxLayout = new QHBoxLayout(); // 将tab水平排布
    hBoxLayout->addWidget(tabWidget);
    this->setLayout(hBoxLayout);
}


// 在“下载标签页”中添加新任务
void filesTabUI::slotCreateNewTask(struct newTaskMessage m)
{
    // 1. 新建Item, 并添加到List
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(QSize(100, 80));
    downloadTab.cList.addItem(item);
    downloadTab.elementPlace << item;

    // 2. 新建文件的Widget控件
    uiItemWidget *itemWidget = new uiItemWidget;
    itemWidget->setDownloadStateUI();
    downloadTab.elementContent << itemWidget;

    // 3. 给Item设置Widget
    downloadTab.cList.setItemWidget(item, itemWidget);
    itemWidget->taskInit(m);


    // 设置删除item的槽
    connect(itemWidget, &uiItemWidget::deleteCompletely, \
                     this, &filesTabUI::deleteCompletely);
    connect(itemWidget, &uiItemWidget::deleteEasyly, \
                     this, &filesTabUI::deleteEasyly);
    connect(itemWidget,  &uiItemWidget::returnTask, \
            this, &filesTabUI::returnTask);
    connect(itemWidget, &uiItemWidget::finishedTask, \
            this, &filesTabUI::finishedTask);

}

// 从Tab中完全移除Item
void filesTabUI::deleteCompletely(void)
{
    // Returns a pointer to the object that sent the signal
    uiItemWidget* itemWidget = qobject_cast<uiItemWidget*>(sender());
    if (itemWidget == nullptr)
        return;

    // 查找位置
    int index = -1;
    int count = 0;

    struct tabList *pList = nullptr;
    if (itemWidget->fManager->fMessage.type == DOWNLOADING)
        pList = &downloadTab;
    else if (itemWidget->fManager->fMessage.type == FINISHED)
        pList = &finishedTab;
    else
        pList = &deletedTab;

    for (auto iter = (*pList).elementContent.begin(); iter != (*pList).elementContent.end(); ++iter)
    {
        if (*iter == itemWidget)
        {
            index = count;
            break;
        }

        count++;
    }
    if (index < 0)
        return;

    // 从List中移除
    (*pList).elementPlace.removeAt(index);
    (*pList).elementContent.removeAt(index);

    // 释放内存
    QListWidgetItem* item = (*pList).cList.takeItem(index);
    delete item;
    itemWidget->deleteLater();
}

// 下载文件->回收站
void filesTabUI::deleteEasyly(void)
{
    // Returns a pointer to the object that sent the signal
    uiItemWidget* itemWidget = qobject_cast<uiItemWidget*>(sender());
    if (itemWidget == nullptr)
        return;

    // 查找位置
    int index = -1;
    int count = 0;
    struct tabList *pList = nullptr;
    if (itemWidget->fManager->fMessage.type == DOWNLOADING)
        pList = &downloadTab;
    else if (itemWidget->fManager->fMessage.type == FINISHED)
        pList = &finishedTab;
    else
        return; // 如果是回收站的文件，按取消就默认还在那里

    for (auto iter = (*pList).elementContent.begin(); iter != (*pList).elementContent.end(); ++iter)
    {
        if (*iter == itemWidget)
        {
            index = count;
            break;
        }
        count++;
    }
    if (index < 0)
        return;

    // 从List中移除
    (*pList).elementPlace.removeAt(index);
    (*pList).elementContent.removeAt(index);

    // 这个采用之前可正常使用
    QListWidgetItem* item = (*pList).cList.takeItem(index);

    // 释放掉之前的，重新新建一个，不然段错误
    uiItemWidget* itemWidgetNew = new uiItemWidget;
    itemWidgetNew->againInit(itemWidget->fManager->fMessage);
    itemWidgetNew->setDeletedStateUI();

    deletedTab.cList.addItem(item);
    deletedTab.elementPlace << item;
    deletedTab.elementContent << itemWidgetNew;
    deletedTab.cList.setItemWidget(item, itemWidgetNew);


    // 设置删除item的槽
    connect(itemWidgetNew, &uiItemWidget::deleteCompletely, \
                     this, &filesTabUI::deleteCompletely);
    connect(itemWidgetNew, &uiItemWidget::deleteEasyly, \
                     this, &filesTabUI::deleteEasyly);
    connect(itemWidgetNew,  &uiItemWidget::returnTask, \
            this, &filesTabUI::returnTask);
    connect(itemWidgetNew, &uiItemWidget::finishedTask, \
            this, &filesTabUI::finishedTask);

    itemWidget->deleteLater();
}

// 回收站->正在下载 、回收站->已完成
void filesTabUI::returnTask()
{
    // Returns a pointer to the object that sent the signal
    uiItemWidget* itemWidget = qobject_cast<uiItemWidget*>(sender());
    if (itemWidget == nullptr)
        return;

    // 查找位置
    int index = -1;
    int count = 0;
    for (auto iter = deletedTab.elementContent.begin(); iter != deletedTab.elementContent.end(); ++iter)
    {
        if (*iter == itemWidget)
        {
            index = count;
            break;
        }

        count++;
    }
    if (index < 0)
        return;

    // 从List中移除
    deletedTab.elementPlace.removeAt(index);
    deletedTab.elementContent.removeAt(index);

    // 这个采用之前可正常使用
    QListWidgetItem* item = deletedTab.cList.takeItem(index);

    // 释放掉之前的，重新新建一个，不然段错误
    uiItemWidget* itemWidgetNew = new uiItemWidget;
    itemWidgetNew->againInit(itemWidget->fManager->fMessage);

    // 通过“现文件大小  == 总大小”判断恢复到哪里
    struct tabList *pList = nullptr;
    if (itemWidget->fManager->fMessage.currentSize == itemWidget->fManager->fMessage.fileSize){
        pList = &finishedTab;
        itemWidgetNew->setFinishedStateUI();
    }
    else{
        pList = &downloadTab;
        itemWidgetNew->setDownloadStateUI();
    }

    (*pList).cList.addItem(item);
    (*pList).elementPlace << item;
    (*pList).elementContent << itemWidgetNew;
    (*pList).cList.setItemWidget(item, itemWidgetNew);

    // 设置删除item的槽
    connect(itemWidgetNew, &uiItemWidget::deleteCompletely, \
                     this, &filesTabUI::deleteCompletely);
    connect(itemWidgetNew, &uiItemWidget::deleteEasyly, \
                     this, &filesTabUI::deleteEasyly);
    connect(itemWidgetNew,  &uiItemWidget::returnTask, \
            this, &filesTabUI::returnTask);
    connect(itemWidgetNew, &uiItemWidget::finishedTask, \
            this, &filesTabUI::finishedTask);

    itemWidget->deleteLater();
}

// 正在下载->已完成
void filesTabUI::finishedTask()
{
    // Returns a pointer to the object that sent the signal
    uiItemWidget* itemWidget = qobject_cast<uiItemWidget*>(sender());
    if (itemWidget == nullptr)
        return;

    // 查找位置
    int index = -1;
    int count = 0;
    for (auto iter = downloadTab.elementContent.begin(); iter != downloadTab.elementContent.end(); ++iter)
    {
        if (*iter == itemWidget)
        {
            index = count;
            break;
        }

        count++;
    }
    if (index < 0)
        return;

    // 从List中移除
    downloadTab.elementPlace.removeAt(index);
    downloadTab.elementContent.removeAt(index);

    // 这个采用之前可正常使用
    QListWidgetItem* item = downloadTab.cList.takeItem(index);

    // 释放掉之前的，重新新建一个，不然段错误
    uiItemWidget* itemWidgetNew = new uiItemWidget;
    itemWidgetNew->againInit(itemWidget->fManager->fMessage);
    itemWidgetNew->setFinishedStateUI();

    finishedTab.cList.addItem(item);
    finishedTab.elementPlace << item;
    finishedTab.elementContent << itemWidgetNew;
    finishedTab.cList.setItemWidget(item, itemWidgetNew);

    // 设置删除item的槽
    connect(itemWidgetNew, &uiItemWidget::deleteCompletely, \
                     this, &filesTabUI::deleteCompletely);
    connect(itemWidgetNew, &uiItemWidget::deleteEasyly, \
                     this, &filesTabUI::deleteEasyly);
    connect(itemWidgetNew, &uiItemWidget::returnTask, \
            this, &filesTabUI::returnTask);
    connect(itemWidgetNew, &uiItemWidget::finishedTask, \
            this, &filesTabUI::finishedTask);

    itemWidget->deleteLater();

}
