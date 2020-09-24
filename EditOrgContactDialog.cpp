/*
 * Класс служит для редактирования организации.
 */

#include "EditOrgContactDialog.h"
#include "ui_EditOrgContactDialog.h"

#include <QSqlQuery>
#include <QMessageBox>
#include <QDesktopWidget>

EditOrgContactDialog::EditOrgContactDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditOrgContactDialog)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() & Qt::WindowMinimizeButtonHint);

    connect(ui->comment, &QTextEdit::textChanged, this, &EditOrgContactDialog::onTextChanged);
    connect(ui->comment, &QTextEdit::cursorPositionChanged, this, &EditOrgContactDialog::onCursorPosChanged);
    connect(ui->backButton, &QAbstractButton::clicked, this, &EditOrgContactDialog::onReturn);
    connect(ui->saveButton, &QAbstractButton::clicked, this, &EditOrgContactDialog::onSave);

    phonesList = { ui->firstNumber, ui->secondNumber, ui->thirdNumber, ui->fourthNumber, ui->fifthNumber };

    QRegularExpression regExp("^[\\+]?[0-9]*$");
    phonesValidator = new QRegularExpressionValidator(regExp, this);

    for (int i = 0; i < phonesList.length(); ++i)
        phonesList.at(i)->setValidator(phonesValidator);

    regExp.setPattern("^[0-9]*$");
    vyborIdValidator = new QRegularExpressionValidator(regExp, this);

    ui->vyborId->setValidator(vyborIdValidator);
}

EditOrgContactDialog::~EditOrgContactDialog()
{
    delete phonesValidator;
    delete vyborIdValidator;
    delete ui;
}

/**
 * Изменяет позицию курсора.
 */
void EditOrgContactDialog::onCursorPosChanged()
{
    if (textCursor.isNull())
    {
        textCursor = ui->comment->textCursor();
        textCursor.movePosition(QTextCursor::End);
    }
    else
        textCursor = ui->comment->textCursor();
}

/**
 * Выполняет обработку совершения операций с привязанным объектом.
 */
bool EditOrgContactDialog::eventFilter(QObject*, QEvent* event)
{
    if (event && event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);

        if (keyEvent && (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab))
        {
            if (ui->comment->hasFocus())
                ui->comment->setTextCursor(textCursor);

            return true;
        }
    }

    return false;
}

/**
 * Реализация закрытия окна и отправка сигнала об позиции окна.
 */
void EditOrgContactDialog::onReturn()
{
    emit sendData(false, this->pos().x(), this->pos().y());

    close();
}

/**
 * Реализация установки позиции окна в зависимости от позиции ViewOrgContactDialog.
 */
void EditOrgContactDialog::setPos(int x, int y)
{
    int nDesktopHeight;
    int nDesktopWidth;
    int nWidgetHeight = QWidget::height();
    int nWidgetWidth = QWidget::width();

    QDesktopWidget desktop;
    QRect rcDesktop = desktop.availableGeometry(this);

    nDesktopWidth = rcDesktop.width();
    nDesktopHeight = rcDesktop.height();

    if (x < 0 && (nDesktopHeight - y) > nWidgetHeight)
    {
        x = 0;
        this->move(x, y);
    }
    else if (x < 0 && ((nDesktopHeight - y) < nWidgetHeight))
    {
        x = 0;
        y = nWidgetHeight;
        this->move(x, y);
    }
    else if ((nDesktopWidth - x) < nWidgetWidth && (nDesktopHeight - y) > nWidgetHeight)
    {
        x = nWidgetWidth;
        this->move(x, y);
    }
    else if ((nDesktopWidth - x) < nWidgetWidth && ((nDesktopHeight - y) < nWidgetHeight))
    {
        x = nWidgetWidth;
        y = nWidgetHeight;
        this->move(x, y);
    }
    else if (x > 0 && ((nDesktopHeight - y) < nWidgetHeight))
    {
        y = nWidgetHeight;
        this->move(x, y);
    }
    else
    {
        this->move(x, y);
    }
}

/**
 * Сохранение введенных данных с проверками.
 */
void EditOrgContactDialog::onSave()
{
    QSqlQuery query(db);

    QString orgName = ui->orgName->text();

    QStringList actualPhonesList;

    for (int i = 0; i < phonesList.length(); ++i)
    {
        if (i < oldPhonesList.length() && phonesList.at(i)->text() == oldPhonesList.at(i))
            actualPhonesList.append(phonesList.at(i)->text());
        else
        {
            phonesList.at(i)->setStyleSheet("border: 1px solid grey");

            actualPhonesList.append(phonesList.at(i)->text().remove(QRegularExpression("^[\\+]?[3]?[8]?")));
        }
    }

    bool empty_field = false;

    if (ui->orgName->text().isEmpty())
    {
         ui->label_15->setText("<span style=\"color: red;\">" + tr("Заполните обязательное поле!") + "</span>");

         ui->orgName->setStyleSheet("border: 1px solid red");

         empty_field = true;
    }
    else
    {
        ui->label_15->setText("");

        ui->orgName->setStyleSheet("border: 1px solid grey");
    }

    if (ui->firstNumber->text().isEmpty())
    {
        ui->label_14->show();
        ui->label_14->setText("<span style=\"color: red;\">" + tr("Заполните обязательное поле!") + "</span>");

        ui->firstNumber->setStyleSheet("border: 1px solid red");

        empty_field = true;
    }
    else
    {
        ui->label_14->setText("");

        ui->firstNumber->setStyleSheet("border: 1px solid grey");
    }

    if (empty_field)
        return;

    bool invalid_phones = false;

    for (int i = 0; i < phonesList.length(); ++i)
    {
        if (!phonesList.at(i)->text().isEmpty())
        {
            bool old_phone = false;

            for (int j = 0; j < oldPhonesList.length(); ++j)
                if (phonesList.at(i)->text() == oldPhonesList.at(j))
                    old_phone = true;

            if (!old_phone)
            {
                QString phone = phonesList.at(i)->text();

                if (isPhone(&phone))
                    phonesList.at(i)->setStyleSheet("border: 1px solid grey");
                else
                {
                    phonesList.at(i)->setStyleSheet("border: 1px solid red");

                    invalid_phones = true;
                }
            }
        }
    }

    if (invalid_phones)
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Номер не соответствует формату!"), QMessageBox::Ok);

        return;
    }

    bool same_phones = false;

    for (int i = 0; i < phonesList.length(); ++i)
        for (int j = 0; j < phonesList.length(); ++j)
        {
            if (!phonesList.at(i)->text().isEmpty() && actualPhonesList.at(i) == actualPhonesList.at(j) && i != j)
            {
                phonesList.at(i)->setStyleSheet("border: 1px solid red");
                phonesList.at(j)->setStyleSheet("border: 1px solid red");

                same_phones = true;
            }
        }

    if (same_phones)
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Присутсвуют одинаковые номера!"), QMessageBox::Ok);

        return;
    }

    bool existing_phones = false;

    for (int i = 0; i < phonesList.length(); ++i)
        if (!phonesList.at(i)->text().isEmpty())
        {
            query.prepare("SELECT EXISTS (SELECT entry_phone FROM entry_phone WHERE entry_phone = '" + actualPhonesList.at(i) + "' AND entry_id <> " + contactId + ")");
            query.exec();
            query.next();

            if (query.value(0) != 0)
            {
                phonesList.at(i)->setStyleSheet("border: 1px solid red");

                existing_phones = true;
            }
        }

    if (existing_phones)
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Введены существующие номера!"), QMessageBox::Ok);

        return;
    }

    query.prepare("UPDATE entry SET entry_type = ?, entry_name = ?, entry_org_name = ?, entry_city = ?, entry_address = ?, "
                  "entry_email = ?, entry_vybor_id = ?, entry_comment = ? WHERE id = ?");
    query.addBindValue("org");
    query.addBindValue(orgName);
    query.addBindValue(orgName);
    query.addBindValue(ui->city->text());
    query.addBindValue(ui->address->text());
    query.addBindValue(ui->email->text());
    query.addBindValue(ui->vyborId->text());
    query.addBindValue(ui->comment->toPlainText().trimmed());
    query.addBindValue(contactId);
    query.exec();

    for (int i = 0; i < phonesList.length(); ++i)
        if (!phonesList.at(i)->text().isEmpty())
        {
            if (i >= oldPhonesList.length())
            {
                query.prepare("INSERT INTO fones (entry_id, fone)"
                               "VALUES(?, ?)");
                query.addBindValue(contactId);
                query.addBindValue(actualPhonesList.at(i));
                query.exec();
            }
            else
            {
                query.prepare("UPDATE fones SET fone = ? WHERE entry_id = ? AND fone = ?");
                query.addBindValue(actualPhonesList.at(i));
                query.addBindValue(contactId);
                query.addBindValue(oldPhonesList.at(i));
                query.exec();
            }
        }

    emit sendData(true, this->pos().x(), this->pos().y());

    close();

    QMessageBox::information(this, tr("Уведомление"), tr("Запись успешно изменена!"), QMessageBox::Ok);
}

/**
 * Реализация проверки введенных данных в поля номеров.
 */
bool EditOrgContactDialog::isPhone(QString* str)
{
    int pos = 0;

    QRegularExpressionValidator validator(QRegularExpression("(^[\\+][3][8][0][0-9]{9}$|^[3][8][0][0-9]{9}$|^[0][0-9]{9}$)"));

    if (validator.validate(*str, pos) == QValidator::Acceptable)
        return true;

    return false;
}

/**
 * Получение и заполнение полей окна необходимыми данными.
 * Получение id контакта из CallHistoryDialog, ViewOrgContactDialog.
 */
void EditOrgContactDialog::setValues(QString id)
{
    contactId = id;

    QSqlQuery query(db);

    query.prepare("SELECT entry_phone FROM entry_phone WHERE entry_id = " + contactId);
    query.exec();

    while (query.next())
        oldPhonesList.append(query.value(0).toString());

    for (int i = 0; i < oldPhonesList.length(); ++i)
        phonesList.at(i)->setText(oldPhonesList.at(i));

    query.prepare("SELECT DISTINCT entry_org_name, entry_city, entry_address, entry_email, entry_vybor_id, "
                  "entry_comment FROM entry WHERE id = " + contactId);
    query.exec();
    query.next();

    ui->orgName->setText(query.value(0).toString());
    ui->city->setText(query.value(1).toString());
    ui->address->setText(query.value(2).toString());
    ui->email->setText(query.value(3).toString());
    ui->vyborId->setText(query.value(4).toString());
    ui->comment->setText(query.value(5).toString());
}

/**
 * Реализация ограничения максимальной длины напоминания 255-ю символами.
 */
void EditOrgContactDialog::onTextChanged()
{
    if (ui->comment->toPlainText().trimmed().length() > 255)
        ui->comment->textCursor().deletePreviousChar();
}

/**
 * Реализация скрытия кнопки возврата к карточке организации.
 */
void EditOrgContactDialog::hideBackButton()
{
    ui->backButton->hide();
}

/**
 * Выполняет обработку нажатий клавиш.
 * Особая обработка для клавиши Enter.
 */
void EditOrgContactDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return)
    {
        if (ui->comment->hasFocus())
            return;
        else
            onSave();
    }
    else
        QDialog::keyPressEvent(event);
}
