#ifndef FILESMANAGER_H
#define FILESMANAGER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QTimer>
#include "../DownloadControler/DownloadController.h"
#include "../UICustomWidgets/newTaskDialogUI.h"

// 文件类型
enum fileType
{
    DOWNLOADING, // 正在下载
    FINISHED, // 已完成
    DELETED, // 回收站
};

// 下载协议
enum agreeMent{
    HTTP,
    FTP,
};

// 保存文件信息
struct fileMessages
{
    QString url; // URL
    unsigned long byteS; // 下载速度(Bytes/s)
    unsigned long limitedSpeed; // 限制速度(Bytes/s)
    int jinDu; // 进度条%
    qint64 fileSize;
    qint64 currentSize;
    int threadNum; // 线程数量
    std::vector<int> thExecuteList; // 每个线程执行了多少Bytes (vector<int>)
    QString savePath; // 保存路径
    QString fileName; // 文件名
    enum fileType type; // 文件类型
    enum agreeMent agMent; // 下载协议
    QString userName;
    QString passWord;
};

class FilesManager : public QObject
{
    Q_OBJECT
public:
    explicit FilesManager(QObject *parent = nullptr);
    ~FilesManager();

    // 添加下载文件
    void addDownloadingFile(struct newTaskMessage);
    DownloadController* m_pDownLoader = nullptr;
    fileMessages fMessage;
    QTimer m_timer;

signals:
    void error(const QString& errorString);
    void downFinished(void);
    void onTimeout(void);

    void readyWrite(void);
};

#endif // FILESMANAGER_H
