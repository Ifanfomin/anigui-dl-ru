#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QDialog>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QProcess>
#include <QMap>

namespace Ui {
class LogWindow;
}

class LogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);
    ~LogWindow();

    void createProcessTab(
        QProcess *process,
        const QString &title
    );

    void appendProcessLog(
        QProcess *process,
        const QString &text
    );

private slots:

    void on_pushButtonClose_clicked();

private:
    Ui::LogWindow *ui;
    QMap<QProcess*, QPlainTextEdit*> processLogs;
};

#endif // LOGWINDOW_H
