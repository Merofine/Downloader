#include "DownloadController.h"
#include "HttpDownloader.h"
#include "FtpDownloader.h"
#include <string>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QtCore>
#include <QtNetwork>
#include <QDebug>

static bool isNetWorkOnline()
{
    QNetworkConfigurationManager mgr;
    return mgr.isOnline();
}

DownloadController::DownloadController(QObject *parent)
: QObject(parent)
{
    m_DownloadCount = 0;
    m_FinishedNum = 0;
    m_FileSize = 0;
    m_File = new QFile;
}

DownloadController::~DownloadController()
{
    //m_File->flush();
    //delete m_File;
//    if(m_tcpSocket != nullptr)
//    {
//        m_tcpSocket->deleteLater();
//    }
//    for(auto i: m_HttpDownloaderList)
//    {
//        i->deleteLater();
//    }
//    for(auto i: m_FtpDownloaderList)
//    {
//        i->deleteLater();
//    }
}

//用阻塞的方式获取下载文件的长度
qint64 DownloadController::GetFileSize(QUrl url)
{
    QNetworkAccessManager manager;
    qDebug() << "Getting the file size...";
    m_Scheme = url.scheme();
    if(m_Scheme == QLatin1String("https") || m_Scheme == QLatin1String("http")){
        QEventLoop loop;
        //发出请求，获取目标地址的头部信息
        QNetworkReply *reply = manager.head(QNetworkRequest(url));
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
        loop.exec();
        QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
        QString fileSize = reply->rawHeader(QString("Content-Length").toUtf8());
        qint64 size = fileSize.toLongLong();
        reply->deleteLater();
        qDebug() << ";The file size is: " << size;
        return size;
    }

    if(m_Scheme == QLatin1String("ftp")){
        //QEventLoop loop;
        QByteArray tmp;
        QFileInfo fileInfo(m_Url.path());
        QString fileName = fileInfo.fileName();
        QString getSizeStr = "SIZE /" + fileName + "\r\n";
        QByteArray ba = getSizeStr.toLocal8Bit();
        m_tcpSocket = new QTcpSocket;
        m_tcpSocket->connectToHost(m_Url.host(), m_Url.port());
        QThread::msleep(300);
        m_tcpSocket->waitForReadyRead();
        tmp = m_tcpSocket->readAll();
        std::string userStr = "USER " +m_Url.userName().toStdString() +"\r\n";
        m_tcpSocket->write(userStr.c_str());
        QThread::msleep(300);
        m_tcpSocket->waitForReadyRead();
        tmp = m_tcpSocket->readAll();
        std::string pwdStr = "PASS " +m_Url.password().toStdString() +"\r\n";
        m_tcpSocket->write(pwdStr.c_str());
        QThread::msleep(300);
        m_tcpSocket->waitForReadyRead();
        tmp = m_tcpSocket->readAll();
        m_tcpSocket->write(ba.data());
        QThread::msleep(300);
        //connect(m_tcpSocket, &QTcpSocket::connected, this, &DownloadController::onConnected);
        //connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
        //connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onConnected()));
        //loop.exec();
        m_tcpSocket->waitForReadyRead();
        QByteArray recv = m_tcpSocket->readAll();
        qDebug() << recv;
        std::string recvStr(recv.data());
        std::string recvSize;
        int temp = recvStr.find("213");
        if(temp < 0){
            //返回0，某些FTP服务器可能不支持SIZE查询大小
            return 0;
        }else{
            recvSize = recvStr.substr(temp + 4);
        }
        qint64 size = std::stoi(recvSize);
        m_tcpSocket->close();
        delete m_tcpSocket;
        return size;
    }

    return -1;
}

void DownloadController::setURL(const QString& url, const QString& username, const QString& password)
{
    QUrl u = QUrl(url);
    m_Scheme = u.scheme();
    if(m_Scheme == QLatin1String("ftp"))
    {
        u.setPort(21);
        if(!username.isEmpty()){
            u.setUserName(username);
        }else{
            // 匿名登录
            u.setUserName("anonymous");
        }
        if(!password.isEmpty()){
            u.setPassword(password);
        }
    }else if(m_Scheme == QLatin1String("http"))
    {
        u.setPort(80);
    }else if(m_Scheme == QLatin1String("https"))
    {
        u.setPort(443);
    }
    m_Url = u;
}

void DownloadController::setDestPath(const QString filePath)
{
    m_filePath = filePath;
}

void DownloadController::startFileDownload(const QString url, int count, int speed, QString userName, QString password)
{
    m_DownloadCount = count;
    m_FinishedNum = 0;
    setURL(url, userName, password);
    m_FileSize = GetFileSize(m_Url);
    m_nCurrentDownloadSize = 0;

    if(!isNetWorkOnline())
    {
        m_Status = t_Error;
        QString ret("Network Unavailable");
        emit error(ret);
        return;
    }

    //获得文件的名字
    QFileInfo fileInfo(m_Url.path());
    QString scheme = m_Url.scheme();
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty())
        fileName = "index.html";//尝试获取index.html

    //m_File->setFileName(QString("D:/test.txt"));
    //打开本地文件
    m_File->setFileName(m_filePath);
    //打开文件
    m_File->open(QIODevice::WriteOnly);
    if(!m_File->isOpen())
    {
        m_Status = t_Error;
        QString ret("FilePath Error");
        emit error(ret);
        return;
    }

    // 每个线程的速度限制
    speed /= count;

    //将文件分成PointCount段，用异步的方式下载
    for(int i = 0;i< m_DownloadCount; i++)
    {
        //先算出每段的开头和结尾（HTTP协议所需要的信息）
        int start = m_FileSize * i / m_DownloadCount;
        int end = m_FileSize * (i+1) / m_DownloadCount;
        if( i != 0 )
            start++;

        if(m_Scheme == QLatin1String("http") || m_Scheme == QLatin1String("https"))
        {
            HttpDownloader *tempDownload = new HttpDownloader(start, end, i+1, speed, nullptr);
            connect(this, SIGNAL(startDownload(QUrl, QFile*)), tempDownload, SLOT(onStart(QUrl, QFile*)));
            connect(this, SIGNAL(stopThreadTask()), tempDownload, SLOT(onStop()));
            connect(this, SIGNAL(restartThreadTask()), tempDownload, SLOT(onRestart()));
            //connect(this, SIGNAL(deleteFileDownload()), tempDownload, SLOT(onDelete()));
            connect(tempDownload, SIGNAL(DownloadFinished()), this, SLOT(SubPartFinished()));
            connect(tempDownload, SIGNAL(DownloadFinished()), tempDownload, SLOT(deleteLater()));
            //connect(tempDownload, SIGNAL(stopThreadRet(int)), this, SLOT(deleteLater()));
            connect(tempDownload, SIGNAL(updateCurrentSize(int,int)), this, SLOT(updateCurrentSize(int,int)));

            //创建线程
            QThread *downloadThread = new QThread;
            m_ThreadList.push_back(downloadThread);
            m_HttpDownloaderList.push_back(tempDownload);


            //将下载器move到线程中
            //connect(downloadThread, SIGNAL(finished()), tempDownload, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), downloadThread, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), this, SLOT(finshedThread()));
            tempDownload->moveToThread(downloadThread);
            downloadThread->start();
            emit startDownload(m_Url, m_File);
        }
        else if(m_Scheme == QLatin1String("ftp"))
        {
            if(m_FileSize <= 0)
            {
                m_Status = t_Error;
                QString ret("Failed to get FileSize from FTP SERVER");
                emit error(ret);
                return;
            }
            FtpDownloader *tempDownload = new FtpDownloader(start, end, i+1, speed, nullptr);
            connect(this, SIGNAL(startDownload(QUrl, QFile*)), tempDownload, SLOT(onStart(QUrl, QFile*)));
            connect(this, SIGNAL(stopThreadTask()), tempDownload, SLOT(onStop()));
            connect(this, SIGNAL(restartThreadTask()), tempDownload, SLOT(onRestart()));
            //connect(this, SIGNAL(deleteFileDownload()), tempDownload, SLOT(onDelete()));
            connect(tempDownload, SIGNAL(DownloadFinished()), this, SLOT(SubPartFinished()));
            connect(tempDownload, SIGNAL(DownloadFinished()), tempDownload, SLOT(deleteLater()));
            connect(tempDownload, SIGNAL(error(QString)), this, SLOT(onError(QString)));
            connect(tempDownload, SIGNAL(updateCurrentSize(int,int)), this, SLOT(updateCurrentSize(int,int)));

            //创建线程
            QThread *downloadThread = new QThread;
            m_ThreadList.push_back(downloadThread);
            m_FtpDownloaderList.push_back(tempDownload);

            //将下载器move到线程中
            //connect(downloadThread, SIGNAL(finished()), tempDownload, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), downloadThread, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), this, SLOT(finshedThread()));
            tempDownload->moveToThread(downloadThread);
            downloadThread->start();
            emit startDownload(m_Url, m_File);

        }
    }
}

void DownloadController::resumeFileDownload(QString url, int threadNum, int speed, std::vector<int> thExcuteList, QString userName, QString password)
{
    m_DownloadCount = threadNum;
    m_FinishedNum = 0;
    setURL(url, userName, password);
    m_FileSize = GetFileSize(m_Url);
    m_nCurrentDownloadSize = 0;
    m_ThreadHadDownload = thExcuteList;

    if(!isNetWorkOnline())
    {
        QString ret("Network Unavailable");
        emit error(ret);
        return;
    }

    //获得文件的名字
    QFileInfo fileInfo(m_Url.path());
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty())
        fileName = "index.html";//尝试获取index.html

    //打开本地文件
    m_File->setFileName(m_filePath);
    //打开文件
    m_File->open(QIODevice::WriteOnly);
    if(!m_File->isOpen())
    {
        QString ret("FilePath Error");
        emit error(ret);
        return;
    }

    // 每个线程的速度限制
    speed /= threadNum;

    //将文件分成PointCount段，用异步的方式下载
    for(int i = 0;i< m_DownloadCount; i++)
    {
        //先算出每段的开头和结尾（HTTP协议所需要的信息）
        int start = m_FileSize * i / m_DownloadCount;
        int end = m_FileSize * (i+1) / m_DownloadCount;
        if( i != 0 )
            start++;

        if(m_Scheme == QLatin1String("http") || m_Scheme == QLatin1String("https"))
        {
            HttpDownloader *tempDownload = new HttpDownloader(start + m_ThreadHadDownload[i], end, i+1, speed, nullptr);
            connect(this, SIGNAL(startDownload(QUrl, QFile*)), tempDownload, SLOT(onStart(QUrl, QFile*)));
            connect(this, SIGNAL(stopThreadTask()), tempDownload, SLOT(onStop()));
            connect(this, SIGNAL(restartThreadTask()), tempDownload, SLOT(onRestart()));
            //connect(this, SIGNAL(deleteFileDownload()), tempDownload, SLOT(onDelete()));
            connect(tempDownload, SIGNAL(DownloadFinished()), this, SLOT(SubPartFinished()));
            connect(tempDownload, SIGNAL(DownloadFinished()), tempDownload, SLOT(deleteLater()));
            //connect(tempDownload, SIGNAL(stopThreadRet(int)), this, SLOT(deleteLater()));
            connect(tempDownload, SIGNAL(updateCurrentSize(int,int)), this, SLOT(updateCurrentSize(int,int)));

            //创建线程
            QThread *downloadThread = new QThread;
            m_ThreadList.push_back(downloadThread);
            m_HttpDownloaderList.push_back(tempDownload);

            //将下载器move到线程中
            //connect(downloadThread, SIGNAL(finished()), tempDownload, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), downloadThread, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), this, SLOT(finshedThread()));
            tempDownload->moveToThread(downloadThread);
            downloadThread->start();
            emit startDownload(m_Url, m_File);
        }
        else if(m_Scheme == QLatin1String("ftp"))
        {
            if(m_FileSize <= 0)
            {
                QString ret("Failed to get FileSize from FTP SERVER");
                emit error(ret);
                return;
            }
            FtpDownloader *tempDownload = new FtpDownloader(start, end, i+1, speed, nullptr);
            connect(this, SIGNAL(startDownload(QUrl, QFile*)), tempDownload, SLOT(onStart(QUrl, QFile*)));
            connect(this, SIGNAL(stopThreadTask()), tempDownload, SLOT(onStop()));
            connect(this, SIGNAL(restartThreadTask()), tempDownload, SLOT(onRestart()));
            //connect(this, SIGNAL(deleteFileDownload()), tempDownload, SLOT(onDelete()));
            connect(tempDownload, SIGNAL(DownloadFinished()), this, SLOT(SubPartFinished()));
            connect(tempDownload, SIGNAL(DownloadFinished()), tempDownload, SLOT(deleteLater()));
            connect(tempDownload, SIGNAL(error(QString)), this, SLOT(onError(QString)));
            connect(tempDownload, SIGNAL(updateCurrentSize(int,int)), this, SLOT(updateCurrentSize(int,int)));

            //创建线程
            QThread *downloadThread = new QThread;
            m_ThreadList.push_back(downloadThread);
            m_FtpDownloaderList.push_back(tempDownload);


            //将下载器move到线程中
            //connect(downloadThread, SIGNAL(finished()), tempDownload, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), downloadThread, SLOT(deleteLater()));
            connect(downloadThread, SIGNAL(finished()), this, SLOT(finshedThread()));
            tempDownload->moveToThread(downloadThread);
            downloadThread->start();
            emit startDownload(m_Url, m_File);
        }
    }
}

void DownloadController::SubPartFinished()
{
    m_FinishedNum++;
    //如果完成数等于文件段数，则说明文件下载完毕，关闭文件，发生信号
    if( m_FinishedNum == m_DownloadCount )
    {
        m_File->flush();
        m_File->close();
        m_File->deleteLater();
        emit fileDownloadFinished();
        qDebug() << "Download finished";
        for(auto i : m_ThreadHadDownload)
        {
            qDebug() << i;
        }
        for(auto i : m_ThreadList)
        {
            i->quit();
        }
    }
}

void DownloadController::onConnected()
{
    qDebug() << "connect!";
    QByteArray recv = m_tcpSocket->readAll();
    qDebug() << recv;
}

void DownloadController::onError(QString errorMsg)
{
    emit error(errorMsg);
}

void DownloadController::onStartTask(QString url, int threadNum, int speed, QString userName, QString password)
{
    startFileDownload(url, threadNum, speed, userName, password);
    m_ThreadHadDownload = std::vector<int>(threadNum, 0);
    m_Status = t_Downloading;
}

void DownloadController::onStopTask()
{
    // 测试代码
   //m_Status = t_Pause;
//   qDebug() << "downloadController stop";
//   if(m_Status != t_Pause)
//   {
//       emit stopThreadTask();
//       m_Status = t_Pause;
//   }else{
//       for(auto i : m_HttpDownloaderList)
//       {
//           i->m_condition.wakeOne();
//           qDebug() << "wakeup";
//       }
//       m_Status = t_Downloading;
//   }

   qDebug() << "downloadController stop";
   emit stopThreadTask();
   m_Status = t_Pause;

   emit stopDownload(m_ThreadHadDownload);
}

void DownloadController::onRestartTask()
{
    m_Status = t_Downloading;
    for(auto i : m_HttpDownloaderList)
    {
        i->m_condition.wakeAll();
        qDebug() << "wakeup";
    }
    for(auto i : m_FtpDownloaderList)
    {
        i->m_condition.wakeAll();
        qDebug() << "wakeup";
    }
    //emit restartThreadTask();
}

void DownloadController::onResumeTask(QString url, int threadNum, int speed, std::vector<int> thExcuteList, QString userName, QString password)
{
    m_nCurrentDownloadSize = 0;
    for(auto i : thExcuteList)
    {
        m_nCurrentDownloadSize += i;
    }
    resumeFileDownload(url, threadNum, speed, thExcuteList, userName, password);
}


void DownloadController::onDeleteTask()
{
    for(auto i : m_ThreadList)
    {
        if(i->isRunning()){
            i->quit();
        }
    }
}

void DownloadController::finshedThread()
{
    qDebug() << "end thread";
}

void DownloadController::updateCurrentSize(int index, int deltaSize)
{
    m_nCurrentDownloadSize += deltaSize;
    m_ThreadHadDownload[index - 1] += deltaSize;
}
