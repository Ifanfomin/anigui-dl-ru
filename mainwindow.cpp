#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(size());

    setWindowTitle("Anime Downloader");

    backendDir = QCoreApplication::applicationDirPath() + "/backend";
    ytdlpDir = QCoreApplication::applicationDirPath();

#ifdef Q_OS_WIN
    backendExe = backendDir + "/backend.exe";
    ytdlpExe = ytdlpDir + "/yt-dlp.exe";
#else

    backendExe = backendDir + "/backend";
    ytdlpExe = ytdlpDir + "/yt-dlp";
#endif

    ui->label_github->setOpenExternalLinks(true);
    ui->label_tg->setOpenExternalLinks(true);

    ui->lineEditIp->setText(
        settings.value("proxy/ip", "").toString()
    );
    ui->lineEditPort->setText(
        settings.value("proxy/port", "").toString()
    );
    ui->lineEditUser->setText(
        settings.value("proxy/user", "").toString()
    );
    ui->lineEditPass->setText(
        settings.value("proxy/pass", "").toString()
    );
    ui->checkBoxSocks->setChecked(
        settings.value("proxy/enabled", false).toBool()
    );

    QString defaultPath =
        QStandardPaths::writableLocation(
            QStandardPaths::DownloadLocation
            ) + "/Anime";

    QString savePath = settings.value(
                                   "download/path",
                                   defaultPath
                                   ).toString();

    ui->lineEditSavePath->setText(savePath);

    logWindow = new LogWindow(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEditInput_returnPressed()
{
    searchText = ui->lineEditInput->text().trimmed();
    if (searchText.isEmpty())
        return;

    animeName = searchText;
    provider = ui->comboBoxProvider->currentText();

    ui->listWidgetAnime->clear();
    ui->listWidgetEpisodesSingle->clear();
    ui->listWidgetEpisodesMany->clear();
    ui->listWidgetSource->clear();
    ui->listWidgetVideo->clear();

    QListWidgetItem *loading = new QListWidgetItem("Поиск...");
    loading->setFlags(Qt::NoItemFlags);
    loading->setForeground(Qt::gray);
    ui->listWidgetAnime->addItem(loading);

    QProcess *process = new QProcess(this);
    QTimer *timer = new QTimer(this);

    timer->setSingleShot(true);

    QStringList args;
    if (proxy) {
        args << "proxy"
             << proxyIp << proxyPort << proxyUser << proxyPass;
    }
    args << "search"
         << provider
         << searchText;

    // таймер вышел
    connect(timer, &QTimer::timeout, this, [=]() {
        process->kill();
        process->deleteLater();
        timer->deleteLater();

        ui->listWidgetAnime->clear();

        QListWidgetItem *msg1 = new QListWidgetItem(
            "Поиск не удался (превышено время ожидания)"
            );
        QListWidgetItem *msg2 = new QListWidgetItem(
            "Попробуйте изменить источник или повторить позже"
            );

        msg1->setFlags(Qt::NoItemFlags);
        msg2->setFlags(Qt::NoItemFlags);
        msg1->setForeground(Qt::gray);
        msg2->setForeground(Qt::gray);

        ui->listWidgetAnime->addItem(msg1);
        ui->listWidgetAnime->addItem(msg2);
    });

    // процесс завершился
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [=](int exitCode, QProcess::ExitStatus status) {

                timer->stop();
                timer->deleteLater();

                ui->listWidgetAnime->clear();

                if (status != QProcess::NormalExit || exitCode != 0) {
                    QListWidgetItem *msg1 = new QListWidgetItem(
                        "Ошибка при выполнении поиска"
                        );
                    QListWidgetItem *msg2 = new QListWidgetItem(
                        "Измените источник или попробуйте позже"
                        );
                    msg1->setFlags(Qt::NoItemFlags);
                    msg1->setForeground(Qt::gray);
                    msg2->setFlags(Qt::NoItemFlags);
                    msg2->setForeground(Qt::gray);
                    ui->listWidgetAnime->addItem(msg1);
                    ui->listWidgetAnime->addItem(msg2);

                    process->deleteLater();
                    return;
                }

                QByteArray output = process->readAllStandardOutput();
                QJsonDocument doc = QJsonDocument::fromJson(output);
                QJsonArray arr = doc.array();

                if (arr.isEmpty()) {
                    QListWidgetItem *msg = new QListWidgetItem(
                        "Пустой ответ от источника"
                        );
                    msg->setFlags(Qt::NoItemFlags);
                    msg->setForeground(Qt::gray);
                    ui->listWidgetAnime->addItem(msg);
                } else {
                    QJsonObject first = arr.first().toObject();
                    if (first["title"].toString() == "empty search result") {

                        QListWidgetItem *msg1 = new QListWidgetItem(
                            "Аниме не найдено, введите название верно"
                            );
                        QListWidgetItem *msg2 = new QListWidgetItem(
                            "Либо измените источник"
                            );

                        msg1->setFlags(Qt::NoItemFlags);
                        msg2->setFlags(Qt::NoItemFlags);
                        msg1->setForeground(Qt::gray);
                        msg2->setForeground(Qt::gray);

                        ui->listWidgetAnime->addItem(msg1);
                        ui->listWidgetAnime->addItem(msg2);
                    } else {
                        for (const QJsonValue &v : arr) {
                            ui->listWidgetAnime->addItem(
                                v.toObject()["title"].toString()
                                );
                        }
                    }
                }

                process->deleteLater();
            });

    // запуск
    process->setWorkingDirectory(backendDir);
    process->start(backendExe, args);
    timer->start(20'000);

}

void MainWindow::on_searchButton_clicked()
{
    on_lineEditInput_returnPressed();
}

void MainWindow::on_listWidgetAnime_itemClicked(QListWidgetItem *item)
{
    animeIndex = ui->listWidgetAnime->row(item);
    animeName = item->text();

    ui->listWidgetEpisodesSingle->clear();
    ui->listWidgetEpisodesMany->clear();
    ui->listWidgetSource->clear();
    ui->listWidgetVideo->clear();
    
    QListWidgetItem *loadingSingle = new QListWidgetItem("Поиск...");
    loadingSingle->setFlags(Qt::NoItemFlags);
    loadingSingle->setForeground(Qt::gray);
    ui->listWidgetEpisodesSingle->addItem(loadingSingle);

    QListWidgetItem *loadingMany = new QListWidgetItem("Поиск...");
    loadingMany->setFlags(Qt::NoItemFlags);
    loadingMany->setForeground(Qt::gray);
    ui->listWidgetEpisodesMany->addItem(loadingMany);

    QProcess *process = new QProcess(this);
    QTimer *timer = new QTimer(this);

    timer->setSingleShot(true);

    QStringList args;
    if (proxy) {
        args << "proxy"
             << proxyIp << proxyPort << proxyUser << proxyPass;
    }
    args << "episodes"
         << provider
         << searchText
         << QString::number(animeIndex);

    // таймер вышел
    connect(timer, &QTimer::timeout, this, [=]() {
        process->kill();
        process->deleteLater();
        timer->deleteLater();

        ui->listWidgetEpisodesSingle->clear();
        ui->listWidgetEpisodesMany->clear();

        QListWidgetItem *msg1 = new QListWidgetItem(
            "Эпизоды не найдены"
            );
        QListWidgetItem *msg2 = new QListWidgetItem(
            "Попробуйте изменить источник или повторить позже"
            );

        msg1->setFlags(Qt::NoItemFlags);
        msg2->setFlags(Qt::NoItemFlags);
        msg1->setForeground(Qt::gray);
        msg2->setForeground(Qt::gray);

        ui->listWidgetEpisodesSingle->addItem(msg1);
        ui->listWidgetEpisodesSingle->addItem(msg2);

        ui->listWidgetEpisodesMany->addItem(msg1);
        ui->listWidgetEpisodesMany->addItem(msg2);
    });

    // процесс завершился
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [=](int exitCode, QProcess::ExitStatus status) {

                timer->stop();
                timer->deleteLater();

                ui->listWidgetEpisodesSingle->clear();
                ui->listWidgetEpisodesMany->clear();

                if (status != QProcess::NormalExit || exitCode != 0) {
                    QListWidgetItem *msg1 = new QListWidgetItem(
                        "Ошибка при поиске эпизодов"
                        );
                    QListWidgetItem *msg2 = new QListWidgetItem(
                        "Измените источник или попробуйте позже"
                        );
                    msg1->setFlags(Qt::NoItemFlags);
                    msg1->setForeground(Qt::gray);
                    msg2->setFlags(Qt::NoItemFlags);
                    msg2->setForeground(Qt::gray);
                    ui->listWidgetEpisodesSingle->addItem(msg1);
                    ui->listWidgetEpisodesSingle->addItem(msg2);

                    ui->listWidgetEpisodesMany->addItem(msg1);
                    ui->listWidgetEpisodesMany->addItem(msg2);

                    process->deleteLater();
                    return;
                }

                QByteArray output = process->readAllStandardOutput();
                QJsonDocument doc = QJsonDocument::fromJson(output);
                QJsonArray arr = doc.array();

                if (arr.isEmpty()) {
                    QListWidgetItem *msg = new QListWidgetItem(
                        "Пустой ответ от источника"
                        );
                    msg->setFlags(Qt::NoItemFlags);
                    msg->setForeground(Qt::gray);
                    ui->listWidgetEpisodesSingle->addItem(msg);
                    ui->listWidgetEpisodesMany->addItem(msg);
                } else {
                    QJsonObject first = arr.first().toObject();
                    if (first["title"].toString() == "empty search result") {

                        QListWidgetItem *msg1 = new QListWidgetItem(
                            "Эпизоды не найдены"
                            );
                        QListWidgetItem *msg2 = new QListWidgetItem(
                            "Измените источник или попробуйте позже"
                            );

                        msg1->setFlags(Qt::NoItemFlags);
                        msg2->setFlags(Qt::NoItemFlags);
                        msg1->setForeground(Qt::gray);
                        msg2->setForeground(Qt::gray);

                        ui->listWidgetEpisodesSingle->addItem(msg1);
                        ui->listWidgetEpisodesSingle->addItem(msg2);

                        ui->listWidgetEpisodesMany->addItem(msg1);
                        ui->listWidgetEpisodesMany->addItem(msg2);
                    } else {
                        for (const QJsonValue &v : arr) {
                            ui->listWidgetEpisodesSingle->addItem(
                                v.toObject()["title"].toString()
                                );
                            ui->listWidgetEpisodesMany->addItem(
                                v.toObject()["title"].toString()
                                );
                        }
                    }
                }

                process->deleteLater();
            });

    // запуск
    process->setWorkingDirectory(backendDir);
    process->start(backendExe, args);
    timer->start(20'000);
}

void MainWindow::on_listWidgetEpisodesSingle_itemClicked(QListWidgetItem *item)
{
    episodesIndexes.clear();
    episodesIndexes.append(ui->listWidgetEpisodesSingle->row(item));

    ui->listWidgetSource->clear();
    ui->listWidgetVideo->clear();

    QListWidgetItem *loading = new QListWidgetItem("Поиск...");
    loading->setFlags(Qt::NoItemFlags);
    loading->setForeground(Qt::gray);
    ui->listWidgetSource->addItem(loading);

    QProcess *process = new QProcess(this);
    QTimer *timer = new QTimer(this);

    timer->setSingleShot(true);

    QStringList args;
    if (proxy) {
        args << "proxy"
             << proxyIp << proxyPort << proxyUser << proxyPass;
    }
    args << "sources"
         << provider
         << searchText
         << QString::number(animeIndex)
         << QString::number(episodesIndexes.last());

    // таймер вышел
    connect(timer, &QTimer::timeout, this, [=]() {
        process->kill();
        process->deleteLater();
        timer->deleteLater();

        ui->listWidgetSource->clear();

        QListWidgetItem *msg1 = new QListWidgetItem(
            "Озвучки не найдены"
            );
        QListWidgetItem *msg2 = new QListWidgetItem(
            "Попробуйте изменить источник или повторить позже"
            );

        msg1->setFlags(Qt::NoItemFlags);
        msg2->setFlags(Qt::NoItemFlags);
        msg1->setForeground(Qt::gray);
        msg2->setForeground(Qt::gray);

        ui->listWidgetSource->addItem(msg1);
        ui->listWidgetSource->addItem(msg2);
    });

    // процесс завершился
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [=](int exitCode, QProcess::ExitStatus status) {

                timer->stop();
                timer->deleteLater();

                ui->listWidgetSource->clear();

                if (status != QProcess::NormalExit || exitCode != 0) {
                    QListWidgetItem *msg1 = new QListWidgetItem(
                        "Ошибка при поиске озвучек"
                        );
                    QListWidgetItem *msg2 = new QListWidgetItem(
                        "Измените источник или попробуйте позже"
                        );
                    msg1->setFlags(Qt::NoItemFlags);
                    msg1->setForeground(Qt::gray);
                    msg2->setFlags(Qt::NoItemFlags);
                    msg2->setForeground(Qt::gray);
                    ui->listWidgetSource->addItem(msg1);
                    ui->listWidgetSource->addItem(msg2);

                    process->deleteLater();
                    return;
                }

                QByteArray output = process->readAllStandardOutput();
                QJsonDocument doc = QJsonDocument::fromJson(output);
                QJsonArray arr = doc.array();

                if (arr.isEmpty()) {
                    QListWidgetItem *msg = new QListWidgetItem(
                        "Пустой ответ от источника"
                        );
                    msg->setFlags(Qt::NoItemFlags);
                    msg->setForeground(Qt::gray);
                    ui->listWidgetSource->addItem(msg);
                } else {
                    QJsonObject first = arr.first().toObject();
                    if (first["title"].toString() == "empty search result") {

                        QListWidgetItem *msg1 = new QListWidgetItem(
                            "Озыучки не найдены"
                            );
                        QListWidgetItem *msg2 = new QListWidgetItem(
                            "Измените источник или попробуйте позже"
                            );

                        msg1->setFlags(Qt::NoItemFlags);
                        msg2->setFlags(Qt::NoItemFlags);
                        msg1->setForeground(Qt::gray);
                        msg2->setForeground(Qt::gray);

                        ui->listWidgetSource->addItem(msg1);
                        ui->listWidgetSource->addItem(msg2);
                    } else {
                        for (const QJsonValue &v : arr) {
                            QJsonObject obj = v.toObject();
                            QString title = obj["title"].toString();
                            QString url = obj["url"].toString();

                            QString item_title;
                            if (url.isEmpty()) {
                                item_title = title;
                            } else {
                                item_title = QString("%1 [плеер] %2").arg(title, url);
                            }
                            QListWidgetItem *item = new QListWidgetItem(item_title);
                            item->setData(Qt::UserRole, title);

                            ui->listWidgetSource->addItem(item);
                        }
                    }
                }

                process->deleteLater();
            });

    // запуск
    process->setWorkingDirectory(backendDir);
    process->start(backendExe, args);
    timer->start(20'000);
}

void MainWindow::on_pushButtonEpisodesMany_clicked()
{
    episodesIndexes.clear();
    QList<QListWidgetItem*> selectedItems = ui->listWidgetEpisodesMany->selectedItems();

    for (QListWidgetItem *item : selectedItems) {
        episodesIndexes.append(
            ui->listWidgetEpisodesMany->row(item));
    }
    ui->listWidgetSource->clear();
    ui->listWidgetVideo->clear();

    QListWidgetItem *loading = new QListWidgetItem("Поиск...");
    loading->setFlags(Qt::NoItemFlags);
    loading->setForeground(Qt::gray);
    ui->listWidgetSource->addItem(loading);

    QProcess *process = new QProcess(this);
    QTimer *timer = new QTimer(this);

    timer->setSingleShot(true);

    QStringList args;
    if (proxy) {
        args << "proxy"
             << proxyIp << proxyPort << proxyUser << proxyPass;
    }
    args << "sources"
         << provider
         << searchText
         << QString::number(animeIndex)
         << QString::number(episodesIndexes.last());

    // таймер вышел
    connect(timer, &QTimer::timeout, this, [=]() {
        process->kill();
        process->deleteLater();
        timer->deleteLater();

        ui->listWidgetSource->clear();

        QListWidgetItem *msg1 = new QListWidgetItem(
            "Озвучки не найдены"
            );
        QListWidgetItem *msg2 = new QListWidgetItem(
            "Попробуйте изменить источник или повторить позже"
            );

        msg1->setFlags(Qt::NoItemFlags);
        msg2->setFlags(Qt::NoItemFlags);
        msg1->setForeground(Qt::gray);
        msg2->setForeground(Qt::gray);

        ui->listWidgetSource->addItem(msg1);
        ui->listWidgetSource->addItem(msg2);
    });

    // процесс завершился
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [=](int exitCode, QProcess::ExitStatus status) {

                timer->stop();
                timer->deleteLater();

                ui->listWidgetSource->clear();

                if (status != QProcess::NormalExit || exitCode != 0) {
                    QListWidgetItem *msg1 = new QListWidgetItem(
                        "Ошибка при поиске озвучек"
                        );
                    QListWidgetItem *msg2 = new QListWidgetItem(
                        "Измените источник или попробуйте позже"
                        );
                    msg1->setFlags(Qt::NoItemFlags);
                    msg1->setForeground(Qt::gray);
                    msg2->setFlags(Qt::NoItemFlags);
                    msg2->setForeground(Qt::gray);
                    ui->listWidgetSource->addItem(msg1);
                    ui->listWidgetSource->addItem(msg2);

                    process->deleteLater();
                    return;
                }

                QByteArray output = process->readAllStandardOutput();
                QJsonDocument doc = QJsonDocument::fromJson(output);
                QJsonArray arr = doc.array();

                if (arr.isEmpty()) {
                    QListWidgetItem *msg = new QListWidgetItem(
                        "Пустой ответ от источника"
                        );
                    msg->setFlags(Qt::NoItemFlags);
                    msg->setForeground(Qt::gray);
                    ui->listWidgetSource->addItem(msg);
                } else {
                    QJsonObject first = arr.first().toObject();
                    if (first["title"].toString() == "empty search result") {

                        QListWidgetItem *msg1 = new QListWidgetItem(
                            "Озыучки не найдены"
                            );
                        QListWidgetItem *msg2 = new QListWidgetItem(
                            "Измените источник или попробуйте позже"
                            );

                        msg1->setFlags(Qt::NoItemFlags);
                        msg2->setFlags(Qt::NoItemFlags);
                        msg1->setForeground(Qt::gray);
                        msg2->setForeground(Qt::gray);

                        ui->listWidgetSource->addItem(msg1);
                        ui->listWidgetSource->addItem(msg2);
                    } else {
                        for (const QJsonValue &v : arr) {
                            QJsonObject obj = v.toObject();
                            QString title = obj["title"].toString();
                            QString url = obj["url"].toString();

                            QString item_title;
                            if (url.isEmpty()) {
                                item_title = title;
                            } else {
                                item_title = QString("%1 [плеер] %2").arg(title, url);
                            }
                            QListWidgetItem *item = new QListWidgetItem(item_title);
                            item->setData(Qt::UserRole, title);

                            ui->listWidgetSource->addItem(item);
                        }
                    }
                }

                process->deleteLater();
            });

    // запуск
    process->setWorkingDirectory(backendDir);
    process->start(backendExe, args);
    timer->start(20'000);
}

void MainWindow::on_listWidgetSource_itemClicked(QListWidgetItem *item)
{
    dubName = item->data(Qt::UserRole).toString();
    sourceIndex = ui->listWidgetSource->row(item);
    ui->listWidgetVideo->clear();

    QListWidgetItem *loading = new QListWidgetItem("Поиск...");
    loading->setFlags(Qt::NoItemFlags);
    loading->setForeground(Qt::gray);
    ui->listWidgetVideo->addItem(loading);

    QProcess *process = new QProcess(this);
    QTimer *timer = new QTimer(this);

    timer->setSingleShot(true);

    QStringList args;
    if (proxy) {
        args << "proxy"
             << proxyIp << proxyPort << proxyUser << proxyPass;
    }
    args << "video_qualities"
         << provider
         << searchText
         << QString::number(animeIndex)
         << QString::number(episodesIndexes.last())
         << QString::number(sourceIndex);

    // таймер вышел
    connect(timer, &QTimer::timeout, this, [=]() {
        process->kill();
        process->deleteLater();
        timer->deleteLater();

        ui->listWidgetVideo->clear();

        QListWidgetItem *msg1 = new QListWidgetItem(
            "Форматы не найдены"
            );
        QListWidgetItem *msg2 = new QListWidgetItem(
            "Попробуйте изменить источник или повторить позже"
            );

        msg1->setFlags(Qt::NoItemFlags);
        msg2->setFlags(Qt::NoItemFlags);
        msg1->setForeground(Qt::gray);
        msg2->setForeground(Qt::gray);

        ui->listWidgetVideo->addItem(msg1);
        ui->listWidgetVideo->addItem(msg2);
    });

    // процесс завершился
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [=](int exitCode, QProcess::ExitStatus status) {

                timer->stop();
                timer->deleteLater();

                ui->listWidgetVideo->clear();

                if (status != QProcess::NormalExit || exitCode != 0) {
                    QListWidgetItem *msg1 = new QListWidgetItem(
                        "Ошибка при поиске формата"
                        );
                    QListWidgetItem *msg2 = new QListWidgetItem(
                        "Измените источник или попробуйте позже"
                        );
                    msg1->setFlags(Qt::NoItemFlags);
                    msg1->setForeground(Qt::gray);
                    msg2->setFlags(Qt::NoItemFlags);
                    msg2->setForeground(Qt::gray);
                    ui->listWidgetVideo->addItem(msg1);
                    ui->listWidgetVideo->addItem(msg2);

                    process->deleteLater();
                    return;
                }

                QByteArray output = process->readAllStandardOutput();
                QJsonDocument doc = QJsonDocument::fromJson(output);
                QJsonArray arr = doc.array();

                if (arr.isEmpty()) {
                    QListWidgetItem *msg = new QListWidgetItem(
                        "Пустой ответ от источника"
                        );
                    msg->setFlags(Qt::NoItemFlags);
                    msg->setForeground(Qt::gray);
                    ui->listWidgetVideo->addItem(msg);
                } else {
                    QJsonObject first = arr.first().toObject();
                    if (first["title"].toString() == "empty search result") {

                        QListWidgetItem *msg1 = new QListWidgetItem(
                            "Форматы не найдены"
                            );
                        QListWidgetItem *msg2 = new QListWidgetItem(
                            "Измените источник или попробуйте позже"
                            );

                        msg1->setFlags(Qt::NoItemFlags);
                        msg2->setFlags(Qt::NoItemFlags);
                        msg1->setForeground(Qt::gray);
                        msg2->setForeground(Qt::gray);

                        ui->listWidgetVideo->addItem(msg1);
                        ui->listWidgetVideo->addItem(msg2);
                    } else {
                        for (const QJsonValue &v : arr) {
                            QJsonObject obj = v.toObject();

                            QString title = QString("Качество: %1p %2").arg(obj["title"].toString(), obj["type"].toString());
                            QString url = obj["url"].toString();
                            QString quality = obj["title"].toString();
                            QString type = obj["type"].toString();

                            QListWidgetItem *item = new QListWidgetItem(title);
                            item->setData(Qt::UserRole, url);
                            item->setData(Qt::UserRole + 1, quality);
                            item->setData(Qt::UserRole + 2, type);

                            ui->listWidgetVideo->addItem(item);
                        }
                    }
                }

                process->deleteLater();
            });

    // запуск
    process->setWorkingDirectory(backendDir);
    process->start(backendExe, args);
    timer->start(20'000);
}

void MainWindow::on_listWidgetVideo_itemClicked(QListWidgetItem *item)
{
    // videoUrl = item->data(Qt::UserRole).toString();
    videoQuality = item->data(Qt::UserRole + 1).toString();
    videoType = item->data(Qt::UserRole + 2).toString();
}

QString MainWindow::buildSocks5Proxy() const
{
    return QString(
               "socks5h://%1:%2@%3:%4"
               ).arg(
            proxyUser,
            proxyPass,
            proxyIp,
            proxyPort
            );
}

void MainWindow::on_downloadButton_clicked()
{
    // надо локально в функции сохранить всё что было указано, и потом можно не волноваться о том что может стереться/измениться

    QList<int> localEpisodesIndexes = episodesIndexes;
    QString localVideoQuality = videoQuality;
    QString localVideoType = videoType;
    int localProcessesCount = ui->spinBoxProcesses->value();
    QString localAnimeName = animeName;
    QString localDubName = dubName;

    QString animeDir;

    if (ui->lineEditSavePath->text().trimmed().isEmpty()) {
        QString downloadsDir =
            QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

        animeDir = downloadsDir + "/Anime";
    } else {
        animeDir = ui->lineEditSavePath->text().trimmed();
    }

    QDir().mkpath(animeDir);

    QList<int> pendingEpisodes = localEpisodesIndexes;

    auto startNextGroup = std::make_shared<std::function<void()>>();

    *startNextGroup = [=, this]() mutable
    {
        if (pendingEpisodes.isEmpty()) {
            qDebug() << "all downloads finished";
            return;
        }

        QList<int> group = pendingEpisodes.mid(0, localProcessesCount);
        QString groupString;
        for (int n : group){
            groupString += QString::number(n) + " ";
        }

        pendingEpisodes = pendingEpisodes.mid(group.size());

        QSharedPointer<int> finishedCount(new int(0));
        int total = group.size();

        // получаем ссылки для скачивания
        QProcess *process = new QProcess(this);
        QStringList args;
        if (proxy) {
            args << "proxy"
                 << proxyIp << proxyPort << proxyUser << proxyPass;
        }
        args << "videos_urls"
             << provider
             << searchText
             << QString::number(animeIndex)
             << groupString
             << QString::number(sourceIndex);

        connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [=](int exitCode, QProcess::ExitStatus status) {

                QByteArray output = process->readAllStandardOutput();
                QJsonDocument doc = QJsonDocument::fromJson(output);
                QJsonArray arr = doc.array();

                QStringList urls;

                for (const QJsonValue &episodeValue : arr) {

                    QJsonArray videos = episodeValue.toArray();

                    QString foundUrl;

                    for (const QJsonValue &videoValue : videos) {

                        QJsonObject obj = videoValue.toObject();

                        if (obj["title"].toString() == localVideoQuality) {
                            foundUrl =
                                obj["url"].toString();

                            break;
                        }
                    }
                    urls << foundUrl;
                }

                for (int i = 0; i < group.size(); i++) {
                    int episodeIndex = group[i];
                    QString videoUrl = urls[i];

                    QString outputTemplate = QString(
                                                 "%1/%2/[%3] %4. %5 %2.%(ext)s"
                                                 ).arg(
                                                     animeDir,
                                                     localAnimeName,
                                                     localDubName,
                                                     QString::number(episodeIndex + 1),
                                                     localVideoQuality
                                                     );

                    QStringList arguments;

                    if (proxy) {
                        arguments << "--proxy" << buildSocks5Proxy();
                    }

                    // общие флаги
                    arguments << "--no-part"
                              << "--continue";

                    if (localVideoType == "m3u8") {
                        arguments << "--merge-output-format" << "mp4";
                    }

                    arguments << "-o" << outputTemplate
                              << videoUrl;

                    // создаём процесс
                    QProcess *process = new QProcess(this);

                    // создаём элемент списка процессов
                    QString title = QString(
                                        "ep %1 | %2 | %3"
                                        ).arg(
                                            QString::number(episodeIndex + 1),
                                            localAnimeName,
                                            localVideoQuality
                                            );

                    // делаем короче
                    QFontMetrics metrics(font());

                    title = metrics.elidedText(
                        title,
                        Qt::ElideMiddle,
                        200
                        );

                    QListWidgetItem *item =
                        new QListWidgetItem(title);

                    // делаем его таб
                    logWindow->createProcessTab(process, title);

                    // сохраняем указатель на процесс
                    item->setData(
                        Qt::UserRole,
                        QVariant::fromValue<quintptr>(
                            reinterpret_cast<quintptr>(process)
                            )
                        );

                    ui->listWidgetProcess->addItem(item);

                    // stdout
                    connect(process,
                            &QProcess::readyReadStandardOutput,
                            this,
                            [this, process]()
                            {
                                QString text = QString::fromUtf8(
                                    process->readAllStandardOutput()
                                    );

                                logWindow->appendProcessLog(process, text);
                            });

                    // stderr
                    connect(process,
                            &QProcess::readyReadStandardError,
                            this,
                            [this, process]()
                            {
                                QString text = QString::fromUtf8(
                                    process->readAllStandardError()
                                    );

                                logWindow->appendProcessLog(process, text);
                            });

                    // завершение процесса
                    connect(process,
                            QOverload<int,
                                      QProcess::ExitStatus>::of(
                                &QProcess::finished),
                            this,
                            [=](int,
                                                 QProcess::ExitStatus) mutable
                            {
                                item->setText("✔ " + item->text());

                                process->deleteLater();

                                (*finishedCount)++;

                                if (*finishedCount >= total) {
                                    (*startNextGroup)();
                                }
                            });

                    // стартуем yt-dlp
                    process->setWorkingDirectory(ytdlpDir);
                    process->start(ytdlpExe, arguments);
                }
        });
        process->setWorkingDirectory(backendDir);
        process->start(backendExe, args);
    };

    (*startNextGroup)();
}

void MainWindow::on_listWidgetProcess_itemClicked(QListWidgetItem *item)
{
    processItem = item;
}

void MainWindow::on_killProcessButton_clicked()
{
    if (!processItem)
        return;

    auto ptr = processItem->data(Qt::UserRole).value<quintptr>();
    QProcess *process = reinterpret_cast<QProcess *>(ptr);

    if (!process)
        return;

    if (process->state() != QProcess::NotRunning) {
        process->kill();               // жёстко останавливаем
        process->waitForFinished(3000);
    }

    processItem->setText(
        "✖ " + processItem->text()
        );

    processItem = nullptr;
}

void MainWindow::on_checkBoxSocks_toggled(bool checked)
{
    proxy = checked;
    settings.setValue("proxy/enabled", checked);
}

void MainWindow::on_lineEditIp_textChanged(const QString &text)
{
    proxyIp = text;
    settings.setValue("proxy/ip", text);
}

void MainWindow::on_lineEditPort_textChanged(const QString &text)
{
    proxyPort = text;
    settings.setValue("proxy/port", text);
}

void MainWindow::on_lineEditUser_textChanged(const QString &text)
{
    proxyUser = text;
    settings.setValue("proxy/user", text);
}

void MainWindow::on_lineEditPass_textChanged(const QString &text)
{
    proxyPass = text;
    settings.setValue("proxy/pass", text);
}

void MainWindow::on_logProcessButton_clicked()
{
    logWindow->show();
}

void MainWindow::on_pushButtonBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Выберите папку",
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
        );

    if (!dir.isEmpty()) {
        ui->lineEditSavePath->setText(dir);
        settings.setValue("download/path", dir);
    }
}

