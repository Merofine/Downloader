#ifndef DownloadController_H
#define DownloadController_H

#include "HttpDownloader.h"
#include "FtpDownloader.h"
#include <QObject>
#include <QUrl>
#include <QFile>
#include <QThread>
#include <QVector>
#include <QTimer>
#include <QAbstractSocket>
#include <QtNetwork>

//下载控制器，界面只需要和控制器
class DownloadController : public QObject
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
    QMutex mutex;   //互斥锁，用于QFile的互斥使用


private:
    QString m_Scheme;                   //协议类型
    QUrl    m_Url;                      //URL地址
    qint8   m_DownloadCount;            //分片数量(线程数量）
    qint8   m_FinishedNum;              //已下载完成分片数
    QString m_filePath;                 //本地文件地址
    QFile   *m_File;                    //QFile
    qint64  m_FileSize;                 //文件大小
    qint64  m_nCurrentDownloadSize;     //已下载文件大小

    DownloadStatus m_Status;                            // 当前下载控制器状态
    std::vector<QThread*> m_ThreadList;                 // 下载器线程数组
    std::vector<HttpDownloader*> m_HttpDownloaderList;  // HTTP下载器数组
    std::vector<FtpDownloader*> m_FtpDownloaderList;    // FTP下载器数组
    std::vector<int> m_ThreadHadDownload;               // 各个下载器线程已下载的字节数
    QTcpSocket *m_tcpSocket;                            // TCP socket，用于FTP查询文件大小

public:
    DownloadController(QObject *parent = nullptr);
    ~DownloadController();
    qint64 GetFileSize(QUrl url);

    // 新建下载任务
    void startFileDownload(const QString url, int count, int speed, QString userName, QString password);
    // 暂停下载任务
    void stopFileDownload();
    // 恢复下载任务（程序重启）
    void resumeFileDownload(QString url, int threadNum, int speed, std::vector<int> thExcuteList, QString userName, QString password);
    // 删除下载任务
    void deleteFileDownload();

    // Scheme
    QString getScheme(){return m_Scheme;}
    // URL地址
    void    setURL(const QString& url, const QString& username = NULL, const QString& password = NULL);
    QString getURL(){return m_Url.url();}
    /*
    // 分片数量
    void    setDownloadCount(const qint8& downloadCount){m_DownloadCount = downloadCount;}
    qint8   getDownloadCount(){return m_DownloadCount;}
    */
    // 文件保存地址
    void    setDestPath(const QString fileName);
    QString getDestPath();
    // 获取当前下载进度
    void getCurrentDownProgress(int& currentSize, int& totalSize){currentSize = m_nCurrentDownloadSize;totalSize = m_FileSize;}
    // 获取当前状态
    DownloadStatus getCurrentStatus(void){return m_Status;}

//信号和槽函数
signals:
    //void FileDownloadFinished();
    void startDownload(QUrl, QFile*);
    void stopDownload(std::vector<int> thExcuteList);
    void stopThreadTask();
    void restartThreadTask();
    void error(QString error);//发给界面
    void fileDownloadFinished();//发给界面
public slots:
    void onStartTask(QString url, int threadNum, int speed, QString userName, QString password);//界面发过来
    void onStopTask();//界面发过来
    void onRestartTask(); //界面发过来
    void onResumeTask(QString url, int threadNum, int speed, std::vector<int> thExcuteList, QString userName, QString password);//继续下载，界面发过来
    void onDeleteTask();//界面发过来
    //void deleteTaskSlot();
    void onConnected();
    void onError(QString errorMsg);
    void SubPartFinished();
    void finshedThread();
    void updateCurrentSize(int,int);
};

#endif // DownloadController_H
