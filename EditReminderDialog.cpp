/*
 * Класс служит для изменения существующего напоминания.
 */

#include "EditReminderDialog.h"
#include "ui_EditReminderDialog.h"

#include <QSqlQuery>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

EditReminderDialog::EditReminderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditReminderDialog)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() & Qt::WindowMinimizeButtonHint);

    my_number = global::getSettingsValue(global::getExtensionNumber("extensions"), "extensions_name").toString();

    connect(ui->textEdit, &QTextEdit::textChanged, this, &EditReminderDialog::onTextChanged);
    connect(ui->textEdit, &QTextEdit::cursorPositionChanged, this, &EditReminderDialog::onCursorPosChanged);
    connect(ui->saveButton, &QAbstractButton::clicked, this, &EditReminderDialog::onSave);
    connect(ui->chooseEmployeeButton, &QAbstractButton::clicked, this, &EditReminderDialog::onChooseEmployee);
}

EditReminderDialog::~EditReminderDialog()
{
    delete ui;
}

/**
 * Получение списка с сотрудниками.
 */
void EditReminderDialog::receiveEmployee(QStringList employee)
{
    this->employee = employee;

    if (employee.length() == 1)
        ui->employee->setText(employee.first());
    else
        ui->employee->setText(tr("Группа") + " (" + QString::number(employee.length()) + ")");
}

/**
 * Реализация выбора сотрудников для рассылки напоминаний.
 */
void EditReminderDialog::onChooseEmployee()
{
    if (employee.isEmpty())
        employee = employeeInitial;

    if (!chooseEmployee.isNull())
        chooseEmployee.data()->close();

    chooseEmployee = new ChooseEmployee;
    chooseEmployee.data()->setValues(employee);
    connect(chooseEmployee.data(), &ChooseEmployee::sendEmployee, this, &EditReminderDialog::receiveEmployee);
    connect(this, &EditReminderDialog::getPos, chooseEmployee, &ChooseEmployee::setPos);
    emit getPos(this->pos().x(), this->pos().y());
    chooseEmployee.data()->show();
    chooseEmployee.data()->setAttribute(Qt::WA_DeleteOnClose);
}

/**
 * Выполняет сохранение напоминания с проверками.
 */
void EditReminderDialog::onSave()
{
    QDateTime dateTime = ui->dateTimeEdit->dateTime();
    QString note = ui->textEdit->toPlainText().trimmed();

    if (dateTime < QDateTime::currentDateTime())
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Указано прошедшее время!"), QMessageBox::Ok);

        return;
    }

    if (note.isEmpty())
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Содержание напоминания не может быть пустым!"), QMessageBox::Ok);

        return;
    }

    if (employee.isEmpty())
        employee = employeeInitial;

    if ((group_id != "0" || (employee.first() == my_number && !ui->chooseEmployeeButton->isEnabled())) && dateTime == oldDateTime && note == oldNote && employee == employeeInitial)
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Для сохранения требуется внести изменения!"), QMessageBox::Ok);

        return;
    }

    QSqlQuery query(db);

    if (ui->chooseEmployeeButton->isHidden())
    {
        if (group_id == "0")
        {
            query.prepare("UPDATE reminders SET datetime = ?, completed = false, active = true WHERE id = ?");
            query.addBindValue(dateTime);
            query.addBindValue(id);
            query.exec();
        }
        else if (group_id != "0")
        {
            query.prepare("UPDATE reminders SET group_id = NULL, datetime = ?, completed = false, active = true WHERE id = ?");
            query.addBindValue(dateTime);
            query.addBindValue(id);
            query.exec();

            if (employee.length() == 2)
            {
                query.prepare("UPDATE reminders SET group_id = NULL WHERE group_id = ?");
                query.addBindValue(group_id);
                query.exec();
            }
        }
    }
    else
    {
        if (employeeInitial.length() == 1 && employee.length() > 1)
        {
            query.prepare("UPDATE reminders SET group_id = ? WHERE id = ?");
            query.addBindValue(id);
            query.addBindValue(id);
            query.exec();

            query.prepare("SELECT group_id FROM reminders WHERE id = ?");
            query.addBindValue(id);
            query.exec();
            query.first();

            group_id = query.value(0).toString();
        }

        if (ui->employee->text() != my_number)
        {
            if (employee == employeeInitial)
            {
                if (employee.length() == 1)
                {
                    query.prepare("UPDATE reminders SET datetime = ?, content = ?, viewed = false, completed = false, active = true WHERE id = ?");
                    query.addBindValue(dateTime);
                    query.addBindValue(note);
                    query.addBindValue(id);
                    query.exec();
                }
                else
                {
                    for (int i = 0; i < employee.length(); ++i)
                    {
                        query.prepare("UPDATE reminders SET datetime = ?, content = ?, viewed = false, completed = false, active = true WHERE phone_to = ? AND group_id = ?");
                        query.addBindValue(dateTime);
                        query.addBindValue(note);
                        query.addBindValue(employee.at(i));
                        query.addBindValue(group_id);
                        query.exec();
                    }
                }
            }
            else
            {
                if (employeeInitial.length() == 1)
                {
                    if (!employee.contains(employeeInitial.first()))
                    {
                        query.prepare("UPDATE reminders SET group_id = NULL, active = false WHERE id = ?");
                        query.addBindValue(id);
                        query.exec();
                    }
                }
                else
                {
                    for (int i = 0; i < employeeInitial.length(); ++i)
                    {
                        if (employee.contains(employeeInitial.at(i)))
                            continue;

                        query.prepare("UPDATE reminders SET group_id = NULL, active = false WHERE phone_to = ? AND group_id = ?");
                        query.addBindValue(employeeInitial.at(i));
                        query.addBindValue(group_id);
                        query.exec();
                    }
                }

                if (employee.length() == 1)
                {
                    if (employeeInitial.contains(employee.first()))
                    {
                        query.prepare("UPDATE reminders SET group_id = NULL, phone_to = ?, datetime = ?, content = ?, viewed = false, completed = false, active = true WHERE group_id = ?");
                        query.addBindValue(employee.first());
                        query.addBindValue(dateTime);
                        query.addBindValue(note);
                        query.addBindValue(group_id);
                        query.exec();
                    }
                    else
                    {
                        query.prepare("INSERT INTO reminders (phone_from, phone_to, datetime, content, viewed, completed, active) VALUES(?, ?, ?, ?, false, false, true)");
                        query.addBindValue(my_number);
                        query.addBindValue(employee.first());
                        query.addBindValue(dateTime);
                        query.addBindValue(note);
                        query.exec();
                    }
                }
                else
                {
                    for (int i = 0; i < employee.length(); ++i)
                    {
                        if (employeeInitial.contains(employee.at(i)))
                        {
                            query.prepare("UPDATE reminders SET datetime = ?, content = ?, viewed = false, completed = false, active = true WHERE phone_to = ? AND group_id = ?");
                            query.addBindValue(dateTime);
                            query.addBindValue(note);
                            query.addBindValue(employee.at(i));
                            query.addBindValue(group_id);
                            query.exec();
                        }
                        else
                        {
                            query.prepare("INSERT INTO reminders (group_id, phone_from, phone_to, datetime, content, viewed, completed, active) VALUES(?, ?, ?, ?, ?, false, false, true)");
                            query.addBindValue(group_id);
                            query.addBindValue(my_number);
                            query.addBindValue(employee.at(i));
                            query.addBindValue(dateTime);
                            query.addBindValue(note);
                            query.exec();
                        }
                    }
                }
            }
        }
        else
        {
            if (employee == employeeInitial)
            {
                query.prepare("UPDATE reminders SET datetime = ?, content = ?, completed = false, active = true WHERE id = ?");
                query.addBindValue(dateTime);
                query.addBindValue(note);
                query.addBindValue(id);
                query.exec();
            }
            else
            {
                if (employeeInitial.length() == 1)
                {
                    if (!employee.contains(employeeInitial.first()))
                    {
                        query.prepare("UPDATE reminders SET group_id = NULL, active = false WHERE id = ?");
                        query.addBindValue(id);
                        query.exec();
                    }
                }
                else
                {
                    for (int i = 0; i < employeeInitial.length(); ++i)
                    {
                        query.prepare("UPDATE reminders SET group_id = NULL, active = false WHERE phone_to = ? AND group_id = ?");
                        query.addBindValue(employeeInitial.at(i));
                        query.addBindValue(group_id);
                        query.exec();
                    }
                }

                if (employeeInitial.contains(employee.first()))
                {
                    query.prepare("UPDATE reminders SET phone_to = ?, datetime = ?, content = ?, completed = false, active = true WHERE id = ?");
                    query.addBindValue(employee.first());
                    query.addBindValue(dateTime);
                    query.addBindValue(note);
                    query.addBindValue(id);
                    query.exec();
                }
                else
                {
                    query.prepare("INSERT INTO reminders (phone_from, phone_to, datetime, content, completed, active) VALUES(?, ?, ?, ?, false, true)");
                    query.addBindValue(employee.first());
                    query.addBindValue(employee.first());
                    query.addBindValue(dateTime);
                    query.addBindValue(note);
                    query.exec();
                }
            }
        }
    }

    emit sendData(true);

    close();

    QMessageBox::information(this, tr("Уведомление"), tr("Напоминание успешно изменено!"), QMessageBox::Ok);
}

/**
 * Выполняет получение идентификатора, времени и содержания заметки
 * из классов PopupReminder и RemindersDialog.
 */
void EditReminderDialog::setValues(QString receivedId, QString receivedGroupId, QDateTime receivedDateTime, QString receivedNote)
{
    id = receivedId;
    group_id = receivedGroupId;
    oldDateTime = receivedDateTime;
    oldNote = receivedNote;

    ui->dateTimeEdit->setDateTime(oldDateTime);
    ui->textEdit->setText(oldNote);

    QSqlQuery query(db);

    if (group_id == "0")
    {
        query.prepare("SELECT phone_from, phone_to FROM reminders WHERE id = ?");
        query.addBindValue(id);
        query.exec();
        query.first();

        QString phone_from = query.value(0).toString();
        QString phone_to = query.value(1).toString();

        employeeInitial.append(phone_to);

        ui->employee->setText(phone_to);

        if (phone_from != phone_to && phone_from != my_number)
        {
            ui->chooseEmployeeButton->hide();

            ui->textEdit->setReadOnly(true);
            ui->textEdit->setStyleSheet("QTextEdit { background-color: #fffff0; }");
        }
    }
    else
    {
        query.prepare("SELECT phone_to FROM reminders WHERE group_id = ?");
        query.addBindValue(group_id);
        query.exec();

        while (query.next())
            employeeInitial.append(query.value(0).toString());

        query.prepare("SELECT phone_from FROM reminders WHERE id = ?");
        query.addBindValue(group_id);
        query.exec();
        query.next();

        if (query.value(0).toString() != my_number)
        {
            ui->chooseEmployeeButton->hide();

            ui->textEdit->setReadOnly(true);
            ui->textEdit->setStyleSheet("QTextEdit {background-color: #fffff0;}");

            ui->employee->setText(my_number);
        }
        else
            ui->employee->setText(tr("Группа") + " (" + QString::number(employeeInitial.length()) + ")");
    }
}

/**
 * Установка начальных значений при закрытии окна.
 */
void EditReminderDialog::closeEvent(QCloseEvent* event)
{
    QDialog::closeEvent(event);

    if (!chooseEmployee.isNull())
        chooseEmployee.data()->close();
}

/**
 * Реализация ограничения максимальной длины напоминания 255-ю символами.
 */
void EditReminderDialog::onTextChanged()
{
    if (ui->textEdit->toPlainText().trimmed().length() > 255)
        ui->textEdit->textCursor().deletePreviousChar();
}

/**
 * Изменяет позицию курсора.
 */
void EditReminderDialog::onCursorPosChanged()
{
    if (textCursor.isNull())
    {
        textCursor = ui->textEdit->textCursor();
        textCursor.movePosition(QTextCursor::End);
    }
    else
        textCursor = ui->textEdit->textCursor();
}

/**
 * Выполняет обработку совершения операций с привязанным объектом.
 */
bool EditReminderDialog::eventFilter(QObject*, QEvent* event)
{
    if (event && event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);

        if (keyEvent && (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab))
        {
            if (ui->textEdit->hasFocus())
                ui->textEdit->setTextCursor(textCursor);

            return true;
        }
    }

    return false;
}

/**
 * Выполняет обработку нажатий клавиш.
 * Особая обработка для клавиши Esc.
 */
void EditReminderDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        QDialog::close();
    else if (event->key() == Qt::Key_Return)
    {
        if (ui->textEdit->hasFocus())
            return;
        else
            onSave();
    }
    else
        QWidget::keyPressEvent(event);
}

/**
 * Реализация добавления к текущему времени 5 минут.
 */
void EditReminderDialog::on_add5MinButton_clicked()
{
    ui->dateTimeEdit->setTime(ui->dateTimeEdit->time().addSecs(300));
}

/**
 * Реализация добавления к текущему времени 10 минут.
 */
void EditReminderDialog::on_add10MinButton_clicked()
{
     ui->dateTimeEdit->setTime(ui->dateTimeEdit->time().addSecs(600));
}

/**
 * Реализация добавления к текущему времени 30 минут.
 */
void EditReminderDialog::on_add30MinButton_clicked()
{
     ui->dateTimeEdit->setTime(ui->dateTimeEdit->time().addSecs(1800));
}

/**
 * Реализация добавления к текущему времени 60 минут.
 */
void EditReminderDialog::on_add60MinButton_clicked()
{
     ui->dateTimeEdit->setTime(ui->dateTimeEdit->time().addSecs(3600));
}
