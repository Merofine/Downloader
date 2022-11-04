#include "newTaskDialogUI.h"

NewTaskDialog::NewTaskDialog(QWidget *parent): QDialog (parent)
{
    // *** 控件的定义及初始化 *** //
    initUI();

    // *** connect *** //
    // 自定义名字复选框勾选or不勾选 ---> 自定义名字or不自定义名字
    connect(nameLabel, &QCheckBox::stateChanged, this, &NewTaskDialog::nameLabelCheckBoxChanged);
    // url内容空or不空 ---> 下载确定按钮不能按or能按
    connect(urlLineEdit, &QLineEdit::textChanged, this, &NewTaskDialog::urlChanged);
    // 按下浏览
    connect(searchDirButton, &QPushButton::pressed, this, &NewTaskDialog::chooseDir);
    // 多线程复选框的勾or不勾 ---> 多线程下载or单线程下载
    connect(multiThreadCheckBox, &QCheckBox::stateChanged, this, &NewTaskDialog::multiThreadCheckBoxChanged);
    // 限速
    connect(limitedSpeedCheckBox, &QCheckBox::stateChanged, this, &NewTaskDialog::limitedSpeedCheckBoxChanged);
    // 按下授权
    connect(ftpCheckbox, &QCheckBox::stateChanged, this, &NewTaskDialog::ftpCheckboxChanged);
    // 按下确定
    connect(confirmButton, &QPushButton::pressed, this, &NewTaskDialog::accept);
    // 按下取消
    connect(cancelButton, &QPushButton::pressed, this, &QDialog::close);
}

void NewTaskDialog::initUI()
{
    urlLabel = new QLabel(tr("下载链接:")); // URL label
    nameLabel = new QCheckBox(tr("自定义文件名:")); // 自定义 checkbox
    nameLabel->setCheckState(Qt::CheckState::Unchecked);
    addrLabel = new QLabel(tr("下载目录:")); // Address label
    errorLabel = new QLabel(tr("   "));
    errorText = new QLabel(tr("该链接非法，本软件只支持ftp和http协议"));
    errorText->setStyleSheet("color:red;");
    errorLabel->hide();
    errorText->hide();

    urlLineEdit = new QLineEdit(); // URL content
    urlLineEdit->setText("https://mirrors.edge.kernel.org/pub/linux/kernel/v5.x/linux-5.10.101.tar.xz"); // https://mirrors.edge.kernel.org/pub/linux/kernel/v1.0/linux-1.0.patch.pl15.gz
    nameLineEdit = new QLineEdit(); // 自定义 content
    nameLineEdit->setEnabled(false);
    addrLineEdit = new QLineEdit(); // Address content
    addrLineEdit->setText(tr("/home/lwb/"));
    searchDirButton = new QPushButton(tr("浏览"));

    multiThreadCheckBox = new QCheckBox; // 多线程复选框
    multiThreadCheckBox->setText(tr("多线程"));
    multiThreadCheckBox->setCheckState(Qt::CheckState::Checked);
    threadNumSpinBox = new QSpinBox; // 多线程数量设置
    threadNumSpinBox->setRange(1,16);
    threadNumSpinBox->setValue(8);

    limitedSpeedCheckBox = new QCheckBox; //是否限速
    limitedSpeedCheckBox->setText(tr("限速(MB/s)"));
    limitedSpeedCheckBox->setCheckState(Qt::CheckState::Unchecked);

    limitedSpeedSpinBox = new QSpinBox; //
    limitedSpeedSpinBox->setRange(1,100);
    limitedSpeedSpinBox->setValue(2);
    limitedSpeedSpinBox->setEnabled(false);

    confirmButton = new QPushButton(tr("确定")); // 确定按钮
    confirmButton->setEnabled(true);

    cancelButton = new QPushButton(tr("取消")); //  取消按钮
    confirmButton->setDefault(true);

    spacer = new QWidget(this); // 空
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    _ftpUI = new QGroupBox;
    ftpUsernameLabel = new QLabel(tr("用户:"));
    ftpPasswordLabel = new QLabel(tr("密码:"));
    ftpUsernameLineEdit = new QLineEdit;
    ftpPasswordLineEdit = new QLineEdit;
    ftpUsernameLineEdit->setEnabled(false);
    ftpPasswordLineEdit->setEnabled(false);
    ftpCheckbox = new QCheckBox;
    ftpCheckbox->setText(tr("Ftp授权"));
    ftpLayout = new QHBoxLayout();
    ftpLayout->addWidget(ftpCheckbox);
    ftpLayout->addWidget(ftpUsernameLabel);
    ftpLayout->addWidget(ftpUsernameLineEdit);
    ftpLayout->addWidget(spacer);
    ftpLayout->addWidget(ftpPasswordLabel);
    ftpLayout->addWidget(ftpPasswordLineEdit);
    _ftpUI->setLayout(ftpLayout);
    _ftpUI->hide();

    // *** 布局 *** //
    labelLayout = new QVBoxLayout();
    labelLayout->addWidget(urlLabel);
    labelLayout->addWidget(errorLabel);
    labelLayout->addWidget(nameLabel);
    labelLayout->addWidget(addrLabel);

    lineEditLayout = new QVBoxLayout();
    lineEditLayout->addWidget(urlLineEdit);
    lineEditLayout->addWidget(errorText);
    lineEditLayout->addWidget(nameLineEdit);

    addressLayout = new QHBoxLayout();
    addressLayout->addWidget(addrLineEdit);
    addressLayout->addWidget(searchDirButton);
    lineEditLayout->addLayout(addressLayout);

    editLayout = new QHBoxLayout();
    editLayout->addLayout(labelLayout);
    editLayout->addLayout(lineEditLayout);

    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(multiThreadCheckBox);
    buttonLayout->addWidget(threadNumSpinBox);
    buttonLayout->addWidget(limitedSpeedCheckBox);
    buttonLayout->addWidget(limitedSpeedSpinBox);
    buttonLayout->addWidget(spacer);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);

    mainLayout = new QVBoxLayout();
    mainLayout->addLayout(editLayout);
    mainLayout->addWidget(_ftpUI);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle("新增下载任务");
}

// 浏览"下载目录"
void NewTaskDialog::chooseDir()
{
    QString tmp = QFileDialog::getExistingDirectory(this);
    if (tmp != ""){
        destDir = tmp;
        addrLineEdit->setText(destDir);
    }
}

// 按下“确定”按键
void NewTaskDialog::accept()
{
    // 获取URL中的文件名
    QString urlString = urlLineEdit->text();
    QString _fileName = "";
    int index = urlString.lastIndexOf("/");
    if (index >= 0)
        _fileName = urlString.right(urlString.length() - index);

    // 是否使用自定义文件名
    _fileName = nameLabel->isChecked() ? nameLineEdit->text() : _fileName;

    // 限制速度
    int _speed;
    _speed = limitedSpeedCheckBox->isChecked() ? limitedSpeedSpinBox->value() : 0;

    // 保存该任务的相关信息
    nTaskMessage.url = urlLineEdit->text();
    nTaskMessage.path = addrLineEdit->text();
    nTaskMessage.fileName = _fileName;
    nTaskMessage.threadNum = threadNumSpinBox->value();
    nTaskMessage.limitedSpeed = _speed*1024*1024;
    nTaskMessage.userName = ftpCheckbox->isChecked() ? ftpUsernameLineEdit->text() : tr("");
    nTaskMessage.password = ftpCheckbox->isChecked() ? ftpPasswordLineEdit->text() : tr("");

    // 判断文件是否存在
    QString orgPath = QDir::cleanPath(nTaskMessage.path + QDir::separator() + nTaskMessage.fileName);
    QFile file(orgPath);
    if (file.exists()){ // 文件存在
        QString dlgTitle="警告";
        QString strInfo="文件已存在, 是否覆盖原来的文件？";
        QMessageBox::StandardButton result;//返回选择的按钮
        result=QMessageBox::question(this, dlgTitle, strInfo,
                          QMessageBox::Yes|QMessageBox::No);
        if (result == QMessageBox::Yes){ // 确定覆盖
            // 隐藏对话框
            QDialog::accept();

            // 添加新的下载任务
            emit signalCreateNewTask(nTaskMessage);

        } else if (result == QMessageBox::No){ // 不处理
            ;
        }
    }else {
        // 保存路径不存在，自动创建
        QDir dir(nTaskMessage.path);
        if (!dir.exists())
            dir.mkdir(nTaskMessage.path);

        // 隐藏对话框
        QDialog::accept();

        // 添加新的下载任务
        emit signalCreateNewTask(nTaskMessage);
    }
}

// 自动检测URL合法性
void NewTaskDialog::urlChanged(QString url) {
    if(url != "" && \
            (url.startsWith("http:") or url.startsWith("https:") or url.startsWith("ftp:"))){

        confirmButton->setEnabled(true);
        setUrlErrorUI(false);

        if (url.startsWith("ftp:")) // 若协议为ftp
            _ftpUI->show();
        else
            _ftpUI->hide();
    }
    else{
        _ftpUI->hide();
        confirmButton->setEnabled(false);
        setUrlErrorUI(true);
    }
}

// 显示URL错误
void NewTaskDialog::setUrlErrorUI(bool key)
{
    if (key) //显示
    {
        errorText->show();
        errorLabel->show();
    }else{
        errorText->hide();
        errorLabel->hide();
    }
}

void NewTaskDialog::nameLabelCheckBoxChanged(int status)
{
    if(status == Qt::CheckState::Checked)
        nameLineEdit->setEnabled(true);
    else
        nameLineEdit->setEnabled(false);
}

void NewTaskDialog::multiThreadCheckBoxChanged(int status)
{
    if(status == Qt::CheckState::Checked)
        threadNumSpinBox->setEnabled(true);
    else
    {
        threadNumSpinBox->setValue(1);
        threadNumSpinBox->setEnabled(false);
    }

}

void NewTaskDialog::limitedSpeedCheckBoxChanged(int status)
{
    if(status == Qt::CheckState::Checked)
        limitedSpeedSpinBox->setEnabled(true);
    else
    {
        limitedSpeedSpinBox->setValue(0);
        limitedSpeedSpinBox->setEnabled(false);
    }
}

void NewTaskDialog::ftpCheckboxChanged(int status)
{
    if(status == Qt::CheckState::Checked){
        ftpUsernameLineEdit->setEnabled(true);
        ftpPasswordLineEdit->setEnabled(true);
    }
    else{
        ftpUsernameLineEdit->setEnabled(false);
        ftpPasswordLineEdit->setEnabled(false);
    }
}
