#include "FtpDownloader.h"
#include "DownloadController.h"
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QSslSocket>
#include <QFile>
#include <QDebug>

static int maxRetry = 0;
QMutex fileMutex;

FtpDownloader::FtpDownloader(int startPoint, int endPoint ,int index, int speed, QObject* parent)
    :QObject(parent), m_Index(index)
{
    m_StartPoint = startPoint;
    m_EndPoint = endPoint;
    m_speed = speed;
    m_HaveDoneBytes = 0;
    m_CurrentRecvBytes = 0;

}

FtpDownloader::~FtpDownloader()
{
    //m_File->deleteLater();
    //m_Qnam->deleteLater();
    //if(m_Reply != nullptr)
    //m_Reply->deleteLater();
}

int FtpDownloader::excuteCmd(std::string cmd) {
    cmd += "\r\n";
    m_commandSocket->write(cmd.c_str());
    return 0;
}

int FtpDownloader::recvControl(int stateCode, std::string errorInfo){
    if(errorInfo.size()==1)
        errorInfo = "state code error!";
    if(nextInfo.size()==0) {
        int t;
        QThread::msleep(50);
        memset(buf, 0, BUFLEN);
        recvInfo.clear();
        m_commandSocket->waitForReadyRead();
        qint64 infolen = m_commandSocket->read(buf, BUFLEN);
        if(infolen==BUFLEN) {
            // 消息过长
           return -1;
        }
        buf[infolen] = '\0';
        t = getStateCode();
        recvInfo = buf;

        int temp = recvInfo.find("\r\n226");
        if(temp>=0) {
            nextInfo = recvInfo.substr(temp+2);
        }
        if(t == stateCode)
            return 0;
        else {
            return -1;
        }
    }
    else {
        recvInfo = nextInfo;
        nextInfo.clear();
        return 0;
    }
}

int FtpDownloader::getStateCode()
{
    int num=0;
    char* p = buf;
    while(p != nullptr)
    {
        num=10*num+(*p)-'0';
        p++;
        if(*p==' ' || *p=='-')
        {
            break;
        }
    }
    return num;
}

int FtpDownloader::getPortNum()
{
    int num1=0,num2=0;

    char* p=buf;
    int cnt=0;
    while( 1 )
    {
        if(cnt == 4 && (*p) != ',')
        {
            if(*p<='9' && *p>='0')
                num1 = 10*num1+(*p)-'0';
        }
        if(cnt == 5)
        {
            if(*p<='9' && *p>='0')
                num2 = 10*num2+(*p)-'0';
        }
        if((*p) == ',')
        {
            cnt++;
        }
        p++;
        if((*p) == ')')
        {
            break;
        }
    }
    return num1*256+num2;
}

int FtpDownloader::intoPasv() {
    //切换到被动模式
    excuteCmd("PASV");
    recvControl(227);
    //executeFTPCmd(227, "PASV");                //227

    //返回的信息格式为---h1,h2,h3,h4,p1,p2
    //其中h1,h2,h3,h4为服务器的地址，p1*256+p2为数据端口
    dataPort = getPortNum();
    //客户端数据传输socket
    m_dataSocket = new QTcpSocket;
    m_dataSocket->connectToHost(m_Url.host(), dataPort);
    return 0;
}

// 获取当前下载进度
void FtpDownloader::getCurrentDownProgress(int& currentSize, int& totalSize)
{
    currentSize = m_HaveDoneBytes;
    totalSize = m_EndPoint - m_StartPoint;
}

// 获取当前状态
FtpDownloader::DownloadStatus FtpDownloader::getCurrentStatus(void)
{
    return m_status;
}

// 开始分片下载
void FtpDownloader::startDownload(const QUrl url)
{
    isDone = false;
    m_Url = url;
    m_userName = std::string(url.userName().toLocal8Bit());
    m_password = std::string(url.password().toLocal8Bit());
    //m_Qnam = new QNetworkAccessManager;

    //qDebug() << "Part " << m_Index << ":" << m_StartPoint << ", " << m_EndPoint <<  "download";

    m_commandSocket = new QTcpSocket;
    m_commandSocket->connectToHost(m_Url.host(), m_Url.port());
    QThread::msleep(300);
    if(0 != recvControl(220)){

    }
    excuteCmd("USER " + m_userName);
    if(0 != recvControl(331)){
        QString msg("FTP Client USER Error");
        emit error(msg);
        return;
    }
    excuteCmd("PASS " + m_password);
    if(0 != recvControl(230)){
        QString msg("FTP Client PASSWORD Error");
        emit error(msg);
        return;
    }
    intoPasv();
    // 设置进入流模式，文件类型为binary
    excuteCmd("TYPE I");
    recvControl(200);
    excuteCmd("MODE S");
    recvControl(200);

    std::string restStr = std::string("REST "+ std::to_string(m_StartPoint));
    std::string retrStr = std::string("RETR "+ m_Url.fileName().toStdString());
    excuteCmd(restStr); // 断点续传
    recvControl(350);
    excuteCmd(retrStr);
    if(0 != recvControl(150))
    {
        QString msg("FTP Client FileName Unavialable");
        emit error(msg);
        return;
    }

    connect(m_dataSocket, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    //connect(m_dataSocket, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    connect(m_commandSocket, SIGNAL(readyRead()), this, SLOT(onControlReadyRead()));

    if(m_speed != 0)
    {
        m_pTimer = new QTimer(this);
        QObject::connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        m_pTimer->start(2000);
    }
}

void FtpDownloader::onStart(QUrl url, QFile* file)
{
    m_File = file;
    startDownload(url);
}

// 暂停下载
void FtpDownloader::onStop()
{
    m_status = t_Pause;
    m_commandSocket->close();
    m_commandSocket->deleteLater();
    m_dataSocket->close();
    m_dataSocket->deleteLater();

    // 使用条件变量使线程进入休眠
    m_mutex.lock();
    m_condition.wait(&m_mutex);
    qDebug() << m_Index << ", " << "restart";
    m_mutex.unlock();

    onRestart();
    m_status = t_Downloading;
}

void FtpDownloader::onRestart()
{
    m_commandSocket = new QTcpSocket;
    m_commandSocket->connectToHost(m_Url.host(), m_Url.port());
    QThread::msleep(300);

    //FTP重连
    recvControl(220);
    excuteCmd("USER " + m_userName);
    recvControl(331);
    excuteCmd("PASS " + m_password);
    recvControl(230);
    intoPasv();
    excuteCmd("TYPE I");
    recvControl(200);
    excuteCmd("MODE S");
    recvControl(200);

    std::string restStr = std::string("REST "+ std::to_string(m_StartPoint + m_HaveDoneBytes));
    std::string retrStr = std::string("RETR "+ m_Url.path().toStdString());
    excuteCmd(restStr); // 断点续传
    recvControl(350);
    excuteCmd(retrStr);
    recvControl(150);

    connect(m_dataSocket, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    //connect(m_dataSocket, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    connect(m_commandSocket, SIGNAL(readyRead()), this, SLOT(onControlReadyRead()));
}

void FtpDownloader::onResume()
{
    // 从序列化中恢复
}

void FtpDownloader::onDelete()
{
    if(m_commandSocket != nullptr)
    {
        m_commandSocket->close();
        m_commandSocket->deleteLater();
    }
    if(m_dataSocket != nullptr)
    {
        m_dataSocket->close();
        m_dataSocket->deleteLater();
    }
}

// 每2s唤醒一次
void FtpDownloader::onTimeout(void)
{
    int delta = m_HaveDoneBytes - m_CurrentRecvBytes;
    m_CurrentRecvBytes = m_HaveDoneBytes;
    //qDebug() << QTime::currentTime().toString(Qt::ISODate) << ": "  << m_Index << ": " << m_Reply->thread()->currentThreadId() \
             << ": " << m_Qnam->thread()->currentThreadId() << ": " << delta;

    //超过限制速度，休眠
    if(delta > m_speed*2)
    {
        double temp = (((delta - m_speed*2)/ (double)m_speed)*1000);
        m_commandSocket->close();
        m_commandSocket->deleteLater();
        m_dataSocket->close();
        m_dataSocket->deleteLater();
        QThread::msleep((int)temp);

        m_commandSocket = new QTcpSocket;
        m_commandSocket->connectToHost(m_Url.host(), m_Url.port());
        QThread::msleep(300);

        //FTP重连
        recvControl(220);
        excuteCmd("USER " + m_userName);
        recvControl(331);
        excuteCmd("PASS " + m_password);
        recvControl(230);
        intoPasv();
        excuteCmd("TYPE I");
        recvControl(200);
        excuteCmd("MODE S");
        recvControl(200);

        std::string restStr = std::string("REST "+ std::to_string(m_StartPoint + m_HaveDoneBytes));
        std::string retrStr = std::string("RETR "+ m_Url.path().toStdString());
        excuteCmd(restStr); // 断点续传
        recvControl(350);
        excuteCmd(retrStr);
        recvControl(150);

        connect(m_dataSocket, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
        connect(m_commandSocket, SIGNAL(readyRead()), this, SLOT(onControlReadyRead()));
    }
}

void FtpDownloader::onControlReadyRead()
{
    // 检测被动模式下 数据Socket是否断开，断开则重连
    m_commandSocket->read(buf, BUFLEN);
    if(getStateCode() == 226 && m_HaveDoneBytes < m_EndPoint - m_StartPoint - 1)
    {
        m_dataSocket->close();
        m_dataSocket->deleteLater();
        intoPasv();
        std::string restStr = std::string("REST "+ std::to_string(m_StartPoint + m_HaveDoneBytes));
        std::string retrStr = std::string("RETR "+ m_Url.path().toStdString());

        excuteCmd(restStr); // 断点续传
        recvControl(350);
        excuteCmd(retrStr);
        recvControl(350);
        connect(m_dataSocket, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
}

void FtpDownloader::onReplyReadyRead()
{
    if(!m_File)
        return;
//    if((m_StartPoint == 0 && m_HaveDoneBytes + m_StartPoint == m_EndPoint) || (m_StartPoint != 0 && m_HaveDoneBytes + m_StartPoint - 1 == m_EndPoint))
//    {
//        onReplyFinished();
//    }else{
        qint64 size = m_dataSocket->read(databuf, DATABUFLEN);
        if(size <= 0)
        {
            qDebug() << "size: " << size;
            return;
        }
        if((m_StartPoint != 0 && size + m_HaveDoneBytes > m_EndPoint - m_StartPoint + 1) || (m_StartPoint == 0 && size + m_HaveDoneBytes > m_EndPoint - m_StartPoint))
        {
            size = m_EndPoint - m_StartPoint + 1 - m_HaveDoneBytes;
        }
        //    QByteArray byteArray = m_dataSocket->readAll();
        //    if (byteArray.size() <= 0)
        //    {
        //          m_status = t_Error;
        //          emit error(tr("Fail Recv File Data"));
        //          return;
        //    }
        fileMutex.lock();
        m_File->seek(m_StartPoint + m_HaveDoneBytes);
        m_File->write(databuf, size);
        m_File->flush();
        fileMutex.unlock();
        m_HaveDoneBytes += size;

        qDebug() << m_Index << " : readyReadh: havedone:" << m_HaveDoneBytes << ", start: " << m_StartPoint << ", end:"<< m_EndPoint;
        if(m_HaveDoneBytes + m_StartPoint == m_EndPoint || m_HaveDoneBytes + m_StartPoint - 1 == m_EndPoint /*|| m_HaveDoneBytes + m_StartPoint + 1 == m_EndPoint*/)
        {
            onReplyFinished();
        }
        emit updateCurrentSize(m_Index, size);
//    }
}

void FtpDownloader::onReplyFinished()
{
    isDone = true;

    qDebug() << m_StartPoint << ", " << m_EndPoint;
    fileMutex.lock();
    m_File->flush();
    fileMutex.unlock();
    m_File = nullptr;
    m_dataSocket->deleteLater();
    m_commandSocket->deleteLater();
    //m_File->close();
    qDebug() << "Part" << m_Index << "download finished";
    emit DownloadFinished();
}


