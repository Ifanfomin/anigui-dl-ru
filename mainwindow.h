#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QSettings>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_lineEditInput_returnPressed();
    void on_searchButton_clicked();
    void on_listWidgetAnime_itemClicked(QListWidgetItem *item);
    void on_listWidgetEpisode_itemClicked(QListWidgetItem *item);
    void on_listWidgetSource_itemClicked(QListWidgetItem *item);
    void on_listWidgetVideo_itemClicked(QListWidgetItem *item);
    void on_downloadButton_clicked();
    void on_listWidgetProcess_itemClicked(QListWidgetItem *item);
    void on_killProcessButton_clicked();

    void on_checkBoxSocks_toggled(bool checked);

    void on_lineEditIp_textChanged(const QString &text);
    void on_lineEditPort_textChanged(const QString &text);
    void on_lineEditUser_textChanged(const QString &text);
    void on_lineEditPass_textChanged(const QString &text);

private:
    Ui::MainWindow *ui;

    QSettings settings;
    QString ytdlpDir;
    QString ytdlpExe;
    QString backendDir;
    QString backendExe;
    QString ytdlpPath;

    QString buildSocks5Proxy() const;
    bool proxy = false;
    QString proxyIp;
    QString proxyPort;
    QString proxyUser;
    QString proxyPass;

    QString provider;
    QString searchText;
    int animeIndex = -1;
    QString animeName;
    int episodeIndex = -1;
    int sourceIndex = -1;
    QString dubName;
    QString videoUrl;
    QString videoQuality;
    QString videoType;

    QListWidgetItem *processItem = nullptr;
};
#endif // MAINWINDOW_H
