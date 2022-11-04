#ifndef UIITEMWIDGET_H
#define UIITEMWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QToolButton>
#include <QProgressBar>
#include <QTimer>
#include <QDialog>
#include "../FileManager/filesmanager.h"
#include "deleteDialogUI.h"
#include "newTaskDialogUI.h"


class uiItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit uiItemWidget(QWidget *parent = nullptr);
    ~uiItemWidget(void);
    FilesManager * fManager;

    // 显示下载路径
    QLabel* m_pDestFileNameTag = nullptr;
    // 显示文本
    QLabel* m_pProgressTag = nullptr;
    //
    QLabel* remainTime = nullptr;
    //
    QLabel* nowSpeed = nullptr;
    //
    QHBoxLayout* fileMessageUI = nullptr;
    // 显示进度
    QProgressBar* m_pProgressBar = nullptr;
    // 播放按钮
    QToolButton* m_pPlayButton = nullptr;
    // Finder按钮
    QToolButton* finderButton = nullptr;
    // 恢复按钮
    QToolButton* returnButton = nullptr;
    // 取消按钮
    QToolButton* cannelButton = nullptr;
    // 空
    QWidget* spacer = nullptr;

    int lastSize = 0;
    int speedtime = 100;
    int nowSpeedS = 0;

    void setDownloadStateUI(void);
    void setFinishedStateUI(void);
    void setDeletedStateUI(void);

    void taskInit(struct newTaskMessage);
    void againInit(fileMessages m);

private:
    deleteDialogUi *delDialog;
    bool firstStart = true;
    void uiInit(void);
    QString getShowTime(int totalTime);

private slots:
    void deleteFully();
    void deleteSimply();

    void onError(QString);
    void onReadyWrite(void);
    void onDownFinished(void);

    void onClickedStartPauseButton(void);
//    void onClickedCannelButton(void);
    void onClickedFinderButton(void);
    void onTimeout(void);

    // todo-list
    void error(QString &error);
    void fileDownloadFinished();

signals:
    void deleteCompletely(void);
    void deleteEasyly(void);
    void returnTask(void);
    void finishedTask(void);

    // todolist
    void onStartTask(QString, int, int, QString, QString);
    void onStopTask();
    void onRestartTask();
    void onResumeTask(QString url, int threadNum, int speed, std::vector<int> thExcuteList, QString, QString);
    void onDeleteTask();

};

#endif // UIITEMWIDGET_H
