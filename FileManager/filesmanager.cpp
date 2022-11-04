#include "filesmanager.h"
#include <QDebug>
#include <QDir>

FilesManager::FilesManager(QObject *parent) : QObject(parent)
{
    // ******** 链接文件下载模块   ******** //
    // 实例化一个下载器对象
    m_pDownLoader = new DownloadController(this);
//    QObject::connect(m_pDownLoader, &HttpDownLoader::error, this, &FilesManager::error);
//    QObject::connect(m_pDownLoader, &HttpDownLoader::readyWrite, this, &FilesManager::readyWrite);
    QObject::connect(m_pDownLoader, &DownloadController::fileDownloadFinished, this, &FilesManager::downFinished);

    // 设置定时器 显示进度
    m_timer.setInterval(10);
    QObject::connect(&m_timer, &QTimer::timeout, this, &FilesManager::onTimeout);
    // ******** 链接UI模块   ******** //


}

FilesManager::~FilesManager(void)
{
    m_pDownLoader->deleteLater();
}

void FilesManager::addDownloadingFile(struct newTaskMessage m)
{
    // 保存文件信息
    fMessage.url = m.url;
    fMessage.byteS = 0;
    fMessage.limitedSpeed = m.limitedSpeed;
    fMessage.jinDu = 0;
    fMessage.threadNum = m.threadNum;
    fMessage.savePath = m.path;
    fMessage.fileName = m.fileName;
    fMessage.type = DOWNLOADING;
    fMessage.agMent = HTTP;
    fMessage.currentSize = 0;
    fMessage.userName = m.userName;
    fMessage.passWord = m.password;
}

