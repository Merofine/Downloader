#include "uiitemwidget.h"
//#include "CustomStyleConfig.h"
#include <QVBoxLayout>
#include <QFileInfo>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFile>
#include "../DownloadControler/DownloadController.h"
#include <QDebug>

uiItemWidget::uiItemWidget(QWidget *parent) : QWidget(parent)
{
    // ******** UI初始化   ******** //
    uiInit();
    connect(m_pPlayButton, &QToolButton::clicked, this, &uiItemWidget::onClickedStartPauseButton); // 下载
    connect(finderButton, &QToolButton::clicked, this, &uiItemWidget::onClickedFinderButton); // 所在文件夹
    connect(returnButton, &QToolButton::clicked, this, &uiItemWidget::returnTask);
    connect(cannelButton, &QToolButton::clicked, delDialog, &deleteDialogUi::openDialog); // 删除
    connect(delDialog, &deleteDialogUi::Confirm, this, &uiItemWidget::deleteFully); // 按下删除对话框的确定
    connect(delDialog, &deleteDialogUi::Cancel, this, &uiItemWidget::deleteSimply); // 按下删除对话框的取消

    // ******** 链接文件管理模块   ******** //
    // 实例化一个文件管理对象
    fManager = new FilesManager;
    connect(fManager, &FilesManager::downFinished, this, &uiItemWidget::onDownFinished);
    connect(&(fManager->m_timer), &QTimer::timeout, this, &uiItemWidget::onTimeout); // 进度条关联

    connect(this, &uiItemWidget::onStartTask, fManager->m_pDownLoader, &DownloadController::onStartTask);
    connect(this, &uiItemWidget::onStopTask, fManager->m_pDownLoader, &DownloadController::onStopTask);
    connect(this, &uiItemWidget::onRestartTask, fManager->m_pDownLoader, &DownloadController::onRestartTask);
    connect(this, &uiItemWidget::onResumeTask, fManager->m_pDownLoader, &DownloadController::onResumeTask);
    connect(this, &uiItemWidget::onDeleteTask, fManager->m_pDownLoader, &DownloadController::onDeleteTask);
    connect(fManager->m_pDownLoader, &DownloadController::error, this, &uiItemWidget::onError);
}

uiItemWidget::~uiItemWidget(void)
{
    fManager->deleteLater();
    m_pDestFileNameTag->deleteLater();
    m_pProgressTag->deleteLater();
    m_pProgressBar->deleteLater();
    m_pPlayButton->deleteLater();
    finderButton->deleteLater();
    returnButton->deleteLater();
    cannelButton->deleteLater();
    delDialog->deleteLater();
}

void uiItemWidget::againInit(fileMessages m)
{
    this->fManager->fMessage = m;

    m_pProgressBar->setValue(m.currentSize * 1.0 / m.fileSize * 100);

    QString downloadStr = "%1 / %2";
    downloadStr = downloadStr.arg(m.currentSize).arg(m.fileSize);
    m_pProgressTag->setText(downloadStr);

    QString orgPath = QDir::cleanPath(m.savePath + QDir::separator() + m.fileName);
    m_pDestFileNameTag->setText(orgPath);

    QUrl _url = QUrl(m.url);
    fManager->fMessage.fileSize = fManager->m_pDownLoader->GetFileSize(_url);

    m_pPlayButton->setEnabled(true);
}


void uiItemWidget::error(QString &s)
{

}

void uiItemWidget::fileDownloadFinished()
{

}

// 新下载任务的UI初始化
void uiItemWidget::taskInit(struct newTaskMessage m)
{
    fManager->addDownloadingFile(m);
    QString orgPath = QDir::cleanPath(m.path + QDir::separator() + m.fileName);
    m_pDestFileNameTag->setText(orgPath);

    QUrl _url = QUrl(m.url);
    fManager->fMessage.fileSize = fManager->m_pDownLoader->GetFileSize(_url);
    QString str = tr("Total Size is ") + QString::number(fManager->fMessage.fileSize) + "B";
    m_pProgressTag->setText(str);
    fManager->m_pDownLoader->setDestPath(orgPath);

    m_pPlayButton->setEnabled(true);
}

void uiItemWidget::uiInit(void)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(4);
    mainLayout->setSpacing(0);

    // 添加路径显示
    m_pDestFileNameTag = new QLabel("");
    mainLayout->addWidget(m_pDestFileNameTag);

    // 添加进度条显示
    QWidget* sliderWidget = new QWidget();
    mainLayout->addWidget(sliderWidget);
    QHBoxLayout* sliderLayout = new QHBoxLayout(sliderWidget);
    sliderLayout->setMargin(0);
    sliderLayout->setSpacing(2);
    m_pProgressBar = new QProgressBar();
    m_pProgressBar->setMinimum(0);
    m_pProgressBar->setMaximum(100);
    sliderLayout->addWidget(m_pProgressBar);

    // 添加恢复按钮
    returnButton = new QToolButton;
    returnButton->setFixedSize(25, 25);
    returnButton->setIcon(QIcon(":./images/n_return.png"));
    returnButton->setToolTip(tr("恢复文件"));
    sliderLayout->addWidget(returnButton);

    // 添加下载/暂停按钮
    m_pPlayButton = new QToolButton;
    m_pPlayButton->setFixedSize(25, 25);
    m_pPlayButton->setIcon(QIcon(":./images/n_play.png"));
    m_pPlayButton->setToolTip(tr("下载/暂停"));
    m_pPlayButton->setEnabled(false);
    sliderLayout->addWidget(m_pPlayButton);

    // 添加Finder按钮
    finderButton = new QToolButton;
    finderButton->setFixedSize(25, 25);
    finderButton->setIcon(QIcon(":./images/n_openFile.png"));
    finderButton->setToolTip(tr("打开文件夹"));
    sliderLayout->addWidget(finderButton);

    // 添加取消按钮
    cannelButton = new QToolButton;
    cannelButton->setFixedSize(25, 25);
    cannelButton->setIcon(QIcon(":./images/n_close.png"));
    cannelButton->setToolTip(tr("删除"));
    sliderLayout->addWidget(cannelButton);

    // 添加下载等相关信息
    fileMessageUI = new QHBoxLayout;
    m_pProgressTag = new QLabel("Loading Url ...");
    remainTime = new QLabel("");
    nowSpeed = new QLabel("");
    remainTime->hide();
    nowSpeed->hide();
    spacer = new QWidget; // 空
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    fileMessageUI->addWidget(m_pProgressTag);
    fileMessageUI->addWidget(spacer);
    fileMessageUI->addWidget(remainTime);
    fileMessageUI->addWidget(nowSpeed);

    mainLayout->addLayout(fileMessageUI);

    // 添加删除对话框
    delDialog = new deleteDialogUi(this);
}

// 彻底删除正在下载文件
void uiItemWidget::deleteFully()
{
    delDialog->close();

    qDebug() << "delete fully";
    // 删除源文件
    QString orgPath = QDir::cleanPath(fManager->fMessage.savePath + QDir::separator() + fManager->fMessage.fileName);
    QFile::remove(orgPath);

    // 修改Tab，并释放资源
    emit onDeleteTask(); // delete downloader
    emit deleteCompletely();
}

// 正在下载->回收站
void uiItemWidget::deleteSimply()
{
    delDialog->close();

    qDebug() << "delete simply";

    // 修改Tab，并释放资源
    emit onDeleteTask(); // delete downloader
    emit deleteEasyly();
}

// 隔一段时间 更新进度
void uiItemWidget::onTimeout(void)
{
    if (fManager->m_pDownLoader->getCurrentStatus() == DownloadController::t_Downloading)
    {
        // 设置进度文本
        int currentSize = 0, totalSize = 0;
        fManager->m_pDownLoader->getCurrentDownProgress(currentSize, totalSize);
        QString downloadStr = "%1 / %2";
        downloadStr = downloadStr.arg(currentSize).arg(totalSize);
        m_pProgressTag->setText(downloadStr);

        fManager->fMessage.currentSize = currentSize;
        // 设置进度条
        if (totalSize > 0)
            m_pProgressBar->setValue(currentSize * 1.0 / totalSize * 100);

        // 计算速度
        speedtime--;
        if (speedtime == 0)
        {
            speedtime = 100; // 定时器10ms定时，这里100次即1s
            float speed = currentSize - lastSize;
            nowSpeedS = (int)speed; // 用于计算剩余时间
            int danwei = 0;
            QString danweiStr;
            while(1)
            {
                if ((speed / 1024) > 0.1)
                {
                    speed = speed/1024;
                    danwei++;
                }else if(danwei > 6){ // 防止死循环太多
                    break;
                }else{
                    break;
                }
            }
            switch (danwei) {
                case 0:
                    danweiStr = "B/s";
                    break;
                case 1:
                    danweiStr = "KB/s";
                    break;
                case 2:
                    danweiStr = "MB/s";
                    break;
                case 3:
                    danweiStr = "GB/s";
                    break;
                default:
                    break;
            }
            QString speedStr = "%1 %2";
            speedStr = speedStr.arg(QString::number(speed,'f', 2)).arg(danweiStr);
            nowSpeed->setText(speedStr);
            lastSize = currentSize;


            // 计算剩余时间
            QString remainTimeStr = "剩余时间 %1        ";
            int time_l = nowSpeedS!=0 ? (totalSize - currentSize)/nowSpeedS : 1; // 防止除数为0
            QString tmpStr = getShowTime(time_l);
            remainTimeStr = remainTimeStr.arg(tmpStr);
            remainTime->setText(remainTimeStr);

        }


        fManager->m_timer.start();
    }
}

QString uiItemWidget::getShowTime(int totalTime)
{
    // 传秒数ss=1，传毫秒数ss=1000
    qint64 ss = 1;
    qint64 mi = ss * 60;
    qint64 hh = mi * 60;
    qint64 dd = hh * 24;

    qint64 day = totalTime / dd;
    qint64 hour = (totalTime - day * dd) / hh;
    qint64 minute = (totalTime - day * dd - hour * hh) / mi;
    qint64 second = (totalTime - day * dd - hour * hh - minute * mi) / ss;

    QString hou = QString::number(hour, 10);
    QString min = QString::number(minute, 10);
    QString sec = QString::number(second, 10);

    hou = hou.length() == 1 ? QString("0%1").arg(hou) : hou;
    min = min.length() == 1 ? QString("0%1").arg(min) : min;
    sec = sec.length() == 1 ? QString("0%1").arg(sec) : sec;
    return hou + ":" + min + ":" + sec;
}

void uiItemWidget::onClickedStartPauseButton(void)
{
    QToolButton* toolButton = qobject_cast<QToolButton*>(sender());
    if (fManager->m_pDownLoader->getCurrentStatus() == DownloadController::t_Finished)
    {
        // 打开文件所在目录
        QFileInfo info(m_pDestFileNameTag->text());
        QDesktopServices::openUrl(QUrl(info.absolutePath()));
    }
    else if (fManager->m_pDownLoader->getCurrentStatus() != DownloadController::t_Downloading)
    {
        // 下载
        toolButton->setIcon(QIcon(":./images/n_pause.png"));
        remainTime->show();
        nowSpeed->show();

        fManager->m_timer.start();
        if (firstStart)
        {
            firstStart = false;
            emit onStartTask(fManager->fMessage.url, fManager->fMessage.threadNum, fManager->fMessage.limitedSpeed, \
                               fManager->fMessage.userName, fManager->fMessage.passWord);
        }else{
            emit onRestartTask();
        }

        cannelButton->setEnabled(false); // 下载时不能删除

    }
    else
    {
        // 暂停
        toolButton->setIcon(QIcon(":./images/n_play.png"));
        remainTime->hide();
        nowSpeed->hide();
        emit onStopTask();
        cannelButton->setEnabled(true); // 下载时能删除
        fManager->m_timer.stop();
    }
}

void uiItemWidget::onClickedFinderButton(void)
{
    QDesktopServices::openUrl(QUrl(fManager->fMessage.savePath));
}

void uiItemWidget::setDownloadStateUI()
{
    m_pPlayButton->show();
    finderButton->show();
    cannelButton->show();
    returnButton->hide();
    fManager->fMessage.type = DOWNLOADING;
}

void uiItemWidget::setFinishedStateUI()
{
    m_pPlayButton->hide();
    finderButton->show();
    cannelButton->show();
    returnButton->hide();
    fManager->fMessage.type = FINISHED;
    remainTime->hide();
    nowSpeed->hide();
}

void uiItemWidget::setDeletedStateUI()
{
    m_pPlayButton->hide();
    finderButton->show();
    cannelButton->show();
    returnButton->show();
    fManager->fMessage.type = DELETED;
    remainTime->hide();
    nowSpeed->hide();
}

void uiItemWidget::onError(QString errorString)
{
    fManager->m_timer.stop();
    m_pProgressTag->setText(errorString);
    // 发生错误时，需要做UI上的暂停，不需要发信号给那边了
    m_pPlayButton->setIcon(QIcon(":./images/n_pause.png"));
    remainTime->hide();
    nowSpeed->hide();
}

void uiItemWidget::onReadyWrite(void)
{
    int currentSize = 0, totalSize = 0;
    fManager->m_pDownLoader->getCurrentDownProgress(currentSize, totalSize);
    QString str = tr("Total Size is ") + QString::number(totalSize) + "B";

    m_pProgressTag->setText(str);
}

// 下载完毕
void uiItemWidget::onDownFinished(void)
{
    fManager->fMessage.currentSize = fManager->fMessage.fileSize;
    emit finishedTask();
}



