#ifndef FTPDOWNLOADER_H
#define FTPDOWNLOADER_H

#include <string>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDataStream>
#include <QFile>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>

const int PORT = 21;
const int BUFLEN = 4096;
const int DATABUFLEN = 4096;
const char* const DELIMITER = "\r\n";

class FtpDownloader : public QObject
{
    Q_OBJECT

public:
    enum DownloadStatus
    {
        t_Start,                // 起始状态
        t_Downloading,          // 下载中
        t_Pause,                // 暂停
        t_Finished,             // 下载完成
        t_Error                 // 错误
    };
    QWaitCondition m_condition; // 互斥条件，用于暂停线程
    QMutex         m_mutex;     // 互斥锁，互斥条件用
    std::atomic_bool isStop;
private:
    QNetworkAccessManager *m_Qnam;
    QNetworkReply         *m_Reply;
    QFile                 *m_File;
    QTimer                *m_pTimer;
    QTcpSocket            *m_commandSocket;
    QTcpSocket            *m_dataSocket;

    const int m_Index;              // 下载器编号，控制器用于区分不同线程的下载器
    QUrl      m_Url;                // URL地址
    int       m_speed;              // 最高下载速度
    qint64    m_CurrentRecvBytes;   // 已下载字节数（用于定时器限速）
    qint64    m_HaveDoneBytes;      // 已下载字节数
    qint64    m_StartPoint;         // 下载文件的开始字节
    qint64    m_EndPoint;           // 下载文件的结束字节

    DownloadStatus  m_status = t_Start;

    // 用于socket首发
    std::string     recvInfo;
    std::string     nextInfo;
    char* buf     = new char[BUFLEN];
    char* databuf = new char[DATABUFLEN];
    std::string     m_userName;
    std::string     m_password;
    int             dataPort;
    bool            isDone;

public:
    FtpDownloader(int startPoint, int endPoint, int index, int speed, QObject* parent = nullptr);
    ~FtpDownloader();

    // 开始下载
    void startDownload(QUrl url);
    // 获取当前下载进度
    void getCurrentDownProgress(int& currentSize, int& totalSize);
    // 获取当前状态
    DownloadStatus getCurrentStatus(void);

private:
    // Socket通讯工具方法
    int excuteCmd(std::string cmd);
    int recvControl(int stateCode, std::string errorInfo="0");
    int getStateCode();
    int getPortNum();
    int intoPasv();

// 信号与槽函数
signals:
    void DownloadFinished();
    void error(QString errorString);
    void readyWrite(void);
    void downFinished(void);
    void updateCurrentSize(int,int);
public slots:
    void onStart(QUrl url, QFile* filePath);
    void onStop();
    void onRestart();
    void onResume();
    void onDelete();
    void onControlReadyRead();
    void onReplyReadyRead();
    void onReplyFinished();
    void onTimeout();
};

#endif // FTPDOWNLOADER_H
