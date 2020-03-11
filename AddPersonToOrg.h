#ifndef ADDPERSONTOORG_H
#define ADDPERSONTOORG_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QHeaderView>
#include <QBoxLayout>
#include <QTableView>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QValidator>
#include <QMessageBox>

namespace Ui {
class AddPersonToOrg;
}

class AddPersonToOrg : public QDialog
{
    Q_OBJECT

public:
    explicit AddPersonToOrg(QWidget *parent = 0);
    ~AddPersonToOrg();

signals:
    void sendPersonID(QString &);

protected slots:
    void deleteObjects();
    void onUpdate();
    void onComboBoxListSelected();
    void onComboBoxSelected();
    void getPersonID(const QModelIndex &index);
    void on_searchButton_clicked();
    void searchFunction();

private slots:
    void on_previousButton_clicked();
    void on_nextButton_clicked();
    void on_nextEndButton_clicked();
    void on_previousStartButton_clicked();
    void on_lineEdit_page_returnPressed();
    void on_lineEdit_returnPressed();

private:
    Ui::AddPersonToOrg *ui;
    QSqlQueryModel *query1;
    QValidator *validator;
    QString page;
    int count;
    int remainder;
    QString pages;
    QSqlDatabase db;
    QSqlQuery query;
    QString go;
    QString entry_name;
    QString entry_city;
    QString entry_phone;
    QString entry_comment;
    bool filter;
    QList<QSqlQueryModel*> queries;
};

#endif // ADDPERSONTOORG_H
