#ifndef DELETEDIALOGUI_H
#define DELETEDIALOGUI_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>

class deleteDialogUi : public QDialog
{
    Q_OBJECT
public:
    explicit deleteDialogUi(QWidget *parent = nullptr);
    ~deleteDialogUi(void);

private:
    QPushButton *confirmButton = nullptr;
    QPushButton *cancelButton = nullptr;
    QHBoxLayout *lay = nullptr;

signals:
    void Cancel(void);
    void Confirm(void);

public slots:
    void openDialog();
};

#endif // DELETEDIALOGUI_H
