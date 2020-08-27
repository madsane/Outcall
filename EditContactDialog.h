#ifndef EDITCONTACTDIALOG_H
#define EDITCONTACTDIALOG_H

#include "AddOrgToPerson.h"

#include <QDialog>
#include <QValidator>
#include <QStringList>
#include <QLineEdit>
#include <QKeyEvent>
#include <QPointer>

class ViewContactDialog;

namespace Ui {
class EditContactDialog;
}

class EditContactDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendData(bool, int, int);

public slots:
    void receiveOrgID(QString);

private slots:
    void onSave();
    void onReturn();
    void onTextChanged();

    void on_addOrgButton_clicked();
    void on_deleteOrgButton_clicked();

    bool isInternalPhone(QString* str);
    bool isPhone(QString* str);
    bool isVyborID(QString* str);

    void setPos(int, int);

    void keyPressEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

public:
    explicit EditContactDialog(QWidget *parent = 0);
    ~EditContactDialog();

    void setValuesContacts(QString);

private:
    Ui::EditContactDialog *ui;

    QPointer<AddOrgToPerson> addOrgToPerson;

    QList<QLineEdit*> phonesList;

    QStringList oldPhonesList;

    QValidator* validator;

    QString updateID;
    QString number;  
};

#endif // EDITCONTACTDIALOG_H
