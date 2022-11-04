#include "deleteDialogUI.h"
deleteDialogUi::deleteDialogUi(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("是否永久删除");
    setFixedSize(200, 50);

    confirmButton = new QPushButton(tr("确定")); // 确定按钮
    confirmButton->setEnabled(true);
    connect(confirmButton, &QPushButton::clicked, this, &deleteDialogUi::Confirm);

    cancelButton = new QPushButton(tr("取消")); //  取消按钮
    cancelButton->setDefault(true);
    connect(cancelButton, &QPushButton::clicked, this, &deleteDialogUi::Cancel);

    lay = new QHBoxLayout();
    lay->addWidget(confirmButton);
    lay->addWidget(cancelButton);

    this->setLayout(lay);
    this->hide();
}

deleteDialogUi::~deleteDialogUi(void)
{
    confirmButton->deleteLater();
    cancelButton->deleteLater();
    lay->deleteLater();
}


void deleteDialogUi::openDialog()
{
    this->open();
}
