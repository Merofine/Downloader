#ifndef NEWTASKDIALOG_H
#define NEWTASKDIALOG_H
#include <QtWidgets>

// 新任务的信息
struct newTaskMessage
{
    QString url;
    QString fileName;
    QString path;
    int threadNum;
    unsigned long limitedSpeed;
    QString userName;
    QString password;
};

class NewTaskDialog : public QDialog
{
    Q_OBJECT
private:
    QLabel *urlLabel;
    QCheckBox *nameLabel;
    QLabel *addrLabel;
    QLabel *threadLabel;
    QLabel *errorLabel;
    QLabel *errorText;
    QLabel *ftpUsernameLabel;
    QLabel *ftpPasswordLabel;

    QLineEdit *urlLineEdit;
    QLineEdit *nameLineEdit;
    QLineEdit *addrLineEdit;
    QLineEdit *ftpUsernameLineEdit;
    QLineEdit *ftpPasswordLineEdit;

    QCheckBox *multiThreadCheckBox;
    QCheckBox *limitedSpeedCheckBox;
    QCheckBox *ftpCheckbox;
    QSpinBox *threadNumSpinBox;
    QSpinBox *limitedSpeedSpinBox;

    QWidget *spacer;
    QGroupBox *_ftpUI;

    QPushButton *searchDirButton;
    QPushButton *confirmButton;
    QPushButton *cancelButton;

    QVBoxLayout *labelLayout;
    QVBoxLayout *lineEditLayout;
    QHBoxLayout *editLayout;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
    QHBoxLayout *addressLayout;
    QHBoxLayout *ftpLayout;

    QString destDir;
    struct newTaskMessage nTaskMessage;

    void setUrlErrorUI(bool); // URL自检错误显示
    void initUI();

public:
    NewTaskDialog(QWidget *parent = nullptr);

public slots:
    void accept();
    void urlChanged(QString);
    void nameLabelCheckBoxChanged(int);
    void multiThreadCheckBoxChanged(int);
    void limitedSpeedCheckBoxChanged(int);
    void ftpCheckboxChanged(int);
    void chooseDir();

signals:
    void signalCreateNewTask(struct newTaskMessage);
};

#endif // NEWTASKDIALOG_H
