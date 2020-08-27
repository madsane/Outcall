#ifndef ADDCONTACTDIALOG_H
#define ADDCONTACTDIALOG_H

#include "AddOrgToPerson.h"

#include <QDialog>
#include <QValidator>
#include <QSqlQuery>
#include <QLineEdit>
#include <QKeyEvent>
#include <QPointer>

namespace Ui {
class AddContactDialog;
}

class AddContactDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendData(bool);

public slots:
    void receiveOrgID(QString);

public:
    explicit AddContactDialog(QWidget *parent = 0);
    ~AddContactDialog();

    void setValues(QString);

private slots:
    void onSave();
    void onTextChanged();

    void on_addOrgButton_clicked();
    void on_deleteOrgButton_clicked();

    bool isInternalPhone(QString* str);
    bool isPhone(QString* str);
    bool isVyborID(QString* str);

    void keyPressEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

private:
    Ui::AddContactDialog *ui;

    QPointer<AddOrgToPerson> addOrgToPerson;

    QList<QLineEdit*> phonesList;

    QValidator* validator;
};

#endif // ADDCONTACTDIALOG_H
