#include "logwindow.h"
#include "ui_logwindow.h"

LogWindow::LogWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LogWindow)
{
    ui->setupUi(this);

    setFixedSize(size());

    setWindowTitle("Downloaders Logs");
}

LogWindow::~LogWindow()
{
    delete ui;
}

void LogWindow::createProcessTab(QProcess *process, const QString &title)
{
    QPlainTextEdit *log = new QPlainTextEdit;
    log->setReadOnly(true);

    log->setLineWrapMode(QPlainTextEdit::NoWrap);

    log->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    log->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->tabWidget->addTab(log, title);
    processLogs[process] = log;
}

void LogWindow::appendProcessLog(QProcess *process, const QString &text)
{
    if (!processLogs.contains(process)) {
        return;
    }

    QPlainTextEdit *log = processLogs[process];
    log->appendPlainText(text);
}

void LogWindow::on_pushButtonClose_clicked()
{
    close();
}

