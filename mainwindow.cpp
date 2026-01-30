#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
    ui->listWidgetEpisode->clear();
    ui->listWidgetSource->clear();

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
             << proxyIp << proxyPort << proxyUser << proxyPass
             << "search" << provider << searchText;
    } else {
        args << "search" << provider << searchText;
    }

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

    ui->listWidgetEpisode->clear();
    ui->listWidgetSource->clear();
    ui->listWidgetVideo->clear();

    QListWidgetItem *loading = new QListWidgetItem("Поиск...");
    loading->setFlags(Qt::NoItemFlags);
    loading->setForeground(Qt::gray);
    ui->listWidgetEpisode->addItem(loading);

    QProcess *process = new QProcess(this);
    QTimer *timer = new QTimer(this);

    timer->setSingleShot(true);

    QStringList args;
    if (proxy) {
        args << "proxy"
             << proxyIp << proxyPort << proxyUser << proxyPass
             << "episodes" << provider << QString::number(animeIndex) << searchText;
    } else {
        args << "episodes" << provider << QString::number(animeIndex) << searchText;
    }

    // таймер вышел
    connect(timer, &QTimer::timeout, this, [=]() {
        process->kill();
        process->deleteLater();
        timer->deleteLater();

        ui->listWidgetEpisode->clear();

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

        ui->listWidgetEpisode->addItem(msg1);
        ui->listWidgetEpisode->addItem(msg2);
    });

    // процесс завершился
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [=](int exitCode, QProcess::ExitStatus status) {

                timer->stop();
                timer->deleteLater();

                ui->listWidgetEpisode->clear();

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
                    ui->listWidgetEpisode->addItem(msg1);
                    ui->listWidgetEpisode->addItem(msg2);

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
                    ui->listWidgetEpisode->addItem(msg);
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

                        ui->listWidgetEpisode->addItem(msg1);
                        ui->listWidgetEpisode->addItem(msg2);
                    } else {
                        for (const QJsonValue &v : arr) {
                            ui->listWidgetEpisode->addItem(
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

void MainWindow::on_listWidgetEpisode_itemClicked(QListWidgetItem *item)
{
    episodeIndex = ui->listWidgetEpisode->row(item);
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
             << proxyIp << proxyPort << proxyUser << proxyPass
             << "sources" << provider << QString::number(animeIndex) << searchText << QString::number(episodeIndex);
    } else {
        args << "sources" << provider << QString::number(animeIndex) << searchText << QString::number(episodeIndex);
    }

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
             << proxyIp << proxyPort << proxyUser << proxyPass
             << "videos" << provider << QString::number(animeIndex) << searchText << QString::number(episodeIndex) << QString::number(sourceIndex);
    } else {
        args << "videos" << provider << QString::number(animeIndex) << searchText << QString::number(episodeIndex) << QString::number(sourceIndex);
    }

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

                            QString title = QString("Качество: %1p").arg(obj["title"].toString());
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
    videoUrl = item->data(Qt::UserRole).toString();
    videoQuality = item->data(Qt::UserRole + 1).toString();
    videoType   = item->data(Qt::UserRole + 2).toString();
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
    QString downloadsDir =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    QString animeDir = downloadsDir + "/Anime";
    QDir().mkpath(animeDir);

    QString outputTemplate = QString(
                                 "%1/%2/[%3] %4. %5 %2.%(ext)s"
                                 ).arg(
                                     animeDir,
                                     animeName,
                                     dubName,
                                     QString::number(episodeIndex + 1),
                                     videoQuality
                                     );

    QStringList arguments;

    if (proxy) {
        arguments << "--proxy" << buildSocks5Proxy();
    }

    // общие флаги
    arguments << "--no-part"
              << "--continue";

    if (videoType == "m3u8") {
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
                            animeName,
                            videoQuality
                            );

    QListWidgetItem *item = new QListWidgetItem(title);

    // сохраняем указатель на процесс
    item->setData(
        Qt::UserRole,
        QVariant::fromValue<quintptr>(
            reinterpret_cast<quintptr>(process)
            )
        );

    ui->listWidgetProcess->addItem(item);

    // stdout
    connect(process, &QProcess::readyReadStandardOutput, this, [process]() {
        qDebug().noquote() << process->readAllStandardOutput();
    });

    // stderr
    connect(process, &QProcess::readyReadStandardError, this, [process]() {
        qDebug().noquote() << process->readAllStandardError();
    });

    // завершение процесса
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, item, process](int code, QProcess::ExitStatus) {

                item->setText("✔ " + item->text());

                process->deleteLater();
            });

    // стартуем yt-dlp
    process->setWorkingDirectory(ytdlpDir);
    process->start(ytdlpExe, arguments);
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
