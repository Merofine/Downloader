#include "HttpDownloader.h"
#include "DownloadController.h"
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QSslSocket>
#include <QFile>
#include <QDebug>

static QMutex fileMutex;

HttpDownloader::HttpDownloader(int startPoint, int endPoint ,int index, int speed, QObject* parent)
    :QObject(parent), m_Index(index)
{
    m_StartPoint = startPoint;
    m_EndPoint = endPoint;
    m_speed = speed;
    m_HaveDoneBytes = 0;
    m_CurrentRecvBytes = 0;

}

HttpDownloader::~HttpDownloader()
{
    //m_File->deleteLater();
    //m_Qnam->deleteLater();
    //if(m_Reply != nullptr)
    //m_Reply->deleteLater();
}

// 获取当前下载进度
void HttpDownloader::getCurrentDownProgress(int& currentSize, int& totalSize)
{
    currentSize = m_HaveDoneBytes;
    totalSize = m_EndPoint - m_StartPoint;
}

// 获取当前状态
HttpDownloader::DownloadStatus HttpDownloader::getCurrentStatus(void)
{
    return m_status;
}

// 开始分片下载
void HttpDownloader::startDownload(const QUrl url)
{
    m_Url = url;
    m_Qnam = new QNetworkAccessManager;

    //qDebug() << "Part " << m_Index << ":" << m_StartPoint << ", " << m_EndPoint <<  "download";

    //根据HTTP协议，写入RANGE头部，说明请求文件的范围
    QNetworkRequest qheader;
    qheader.setUrl(url);
    qheader.setRawHeader("User-Agent", "QtApp-HttpDownLoader");
    qheader.setRawHeader("Accept", "application/octet-stream");

    /*//限速，但不能起作用
    qheader.setAttribute(QNetworkRequest::MaximumDownloadBufferSizeAttribute, 1);
    qheader.setAttribute(QNetworkRequest::DownloadBufferAttribute, 1);
    qheader.setAttribute(QNetworkRequest::SynchronousRequestAttribute, false);
    */

    //获取的文件范围
    QString range;
    range.sprintf("bytes=%lld-%lld", m_StartPoint, m_EndPoint);
    qheader.setRawHeader("Range", range.toUtf8());

    //开始下载
    m_Reply = m_Qnam->get(qheader);
    connect(m_Reply, SIGNAL(readyRead()),this, SLOT(onReplyReadyRead()));
    //connect(m_Reply, SIGNAL(finished()),this, SLOT(onReplyFinished()));

    if(m_speed != 0){
        m_pTimer = new QTimer(this);
        QObject::connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        m_pTimer->start(2000);
    }
}

void HttpDownloader::onStart(QUrl url, QFile* file)
{

    m_File = file;
    startDownload(url);
}

// 暂停下载
void HttpDownloader::onStop()
{
    m_status = t_Pause;
    m_Reply->close();
    m_Reply->deleteLater();
    m_mutex.lock();
    m_condition.wait(&m_mutex);
    qDebug() << m_Index << ", " << "restart";
    m_mutex.unlock();
    onRestart();
    m_status = t_Downloading;
}

void HttpDownloader::onRestart()
{
    //m_condition.wakeAll();
    //m_Qnam->deleteLater();
    //m_Qnam = new QNetworkAccessManager;
    //m_Reply->close();
    //根据HTTP协议，写入RANGE头部，说明请求文件的范围
    QNetworkRequest qheader;
    qheader.setUrl(m_Url);
    qheader.setRawHeader("User-Agent", "QtApp-HttpDownLoader");
    qheader.setRawHeader("Accept", "application/octet-stream");
    //获取的文件范围
    QString range;
    range.sprintf("bytes=%lld-%lld", m_StartPoint + m_HaveDoneBytes, m_EndPoint);
    //qDebug() << m_Index << " startDownload: " << range;
    qheader.setRawHeader("Range", range.toUtf8());

    //开始下载
    m_Reply = m_Qnam->get(qheader);
    connect(m_Reply, SIGNAL(readyRead()),this, SLOT(onReplyReadyRead()));
}

void HttpDownloader::onResume()
{
    // 从序列化中恢复
}

void HttpDownloader::onDelete()
{

}

// 每2s唤醒一次
void HttpDownloader::onTimeout(void)
{
    int delta = m_HaveDoneBytes - m_CurrentRecvBytes;
    m_CurrentRecvBytes = m_HaveDoneBytes;
    //qDebug() << QTime::currentTime().toString(Qt::ISODate) << ": "  << m_Index << ": " << m_Reply->thread()->currentThreadId() \
             << ": " << m_Qnam->thread()->currentThreadId() << ": " << delta;

    //超过限制速度，休眠
    if(delta > m_speed*2)
    {
        double temp = (((delta - m_speed*2)/ (double)m_speed)*1000);
        m_Reply->close();
        m_Reply->deleteLater();
        QThread::msleep((int)temp);

        QNetworkRequest qheader;
        qheader.setUrl(m_Url);
        qheader.setRawHeader("User-Agent", "QtApp-HttpDownLoader");
        qheader.setRawHeader("Accept", "application/octet-stream");

        QString range;
        range.sprintf("bytes=%lld-%lld", m_StartPoint + m_HaveDoneBytes, m_EndPoint);
        qheader.setRawHeader("Range", range.toUtf8());
        m_Reply = m_Qnam->get(qheader);
        connect(m_Reply, SIGNAL(readyRead()),this, SLOT(onReplyReadyRead()));
    }
}

void HttpDownloader::onReplyReadyRead()
{
    if(!m_File)
        return;
    QByteArray byteArray = m_Reply->readAll();
    if (byteArray.size() <= 0)
    {
          m_status = t_Error;
          emit error(tr("Fail Recv File Data"));
          return;
    }
    fileMutex.lock();
    m_File->seek(m_StartPoint + m_HaveDoneBytes);
    m_File->write(byteArray);
    m_File->flush();
    fileMutex.unlock();
    m_HaveDoneBytes += byteArray.size();

    if(m_HaveDoneBytes + m_StartPoint == m_EndPoint || m_HaveDoneBytes + m_StartPoint - 1 == m_EndPoint || m_HaveDoneBytes + m_StartPoint + 1 == m_EndPoint)
    {
        onReplyFinished();
    }
    emit updateCurrentSize(m_Index, byteArray.size());
}

void HttpDownloader::onReplyFinished()
{
    fileMutex.lock();
    m_File->flush();
    fileMutex.unlock();
    m_File = nullptr;
    m_Reply->deleteLater();
    qDebug() << "Part" << m_Index << "download finished";
    emit DownloadFinished();
}


