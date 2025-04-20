#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QAudioOutput>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileinfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    m_iCurrentLyricLine(0)
{
    ui->setupUi(this);
    LoadQss(QApplication::applicationDirPath() + "/style.qss");
    ui->label_cover->setMinimumSize(QSize(200, 200));

    m_pPlayer = new QMediaPlayer;
    QAudioOutput* audioOutput = new QAudioOutput;
    m_pPlayer->setAudioOutput(audioOutput);
    connect(m_pPlayer, &QMediaPlayer::positionChanged, ui->playSlider, [this](qint64 position)
    {
        if (m_iCurrentLyricLine <= m_lyrics.size() - 1)
        {
            if (position > m_lyrics[m_iCurrentLyricLine].timeMs)
            {
                ui->label_lyric->setText(m_lyrics[m_iCurrentLyricLine].text);
                ++m_iCurrentLyricLine;
            }
        }
        ui->label_currentTime->setText(GetTimeFormat(position));
        ui->playSlider->setValue(position);
    });
    connect(m_pPlayer, &QMediaPlayer::metaDataChanged, this, [this] 
    {
            QMediaMetaData metaData = m_pPlayer->metaData();
            QVariant durationData = metaData.value(QMediaMetaData::Duration);
            if (durationData.isValid())
            {
                qint64 duration     = durationData.value<qint64>();
                QString strDuration = GetTimeFormat(duration);
                ui->label_totalTime->setText(strDuration);
                ui->playSlider->setRange(0, duration);
            }
            QVariant coverData =  metaData.value(QMediaMetaData::CoverArtImage);
            if (!coverData.isValid())
            {
                coverData = metaData.value(QMediaMetaData::ThumbnailImage);
            }
            if (coverData.isValid())
            {
                QImage image = coverData.value<QImage>();
                if (!image.isNull())
                {
                    m_currentCoverPixmap = QPixmap::fromImage(image);
                    ui->label_cover->setPixmap(m_currentCoverPixmap.scaled(ui->label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
    });
    QFileInfo fileInfo("C:/Users/Ze/Music/王菲-如愿.mp3");
    m_pPlayer->setSource(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    m_lyrics.clear();
    m_lyrics = ParseLrcFile(fileInfo.absolutePath() + "/" + fileInfo.baseName() + ".lrc");
    audioOutput->setVolume(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    if (!m_currentCoverPixmap.isNull())
    {
        ui->label_cover->setPixmap(m_currentCoverPixmap.scaled(ui->label_cover->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    return QMainWindow::resizeEvent(event);
}

QList<LyricLine> MainWindow::ParseLrcFile(const QString& filePath)
{
    QList<LyricLine> lyrics;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        return lyrics;
    }

    QTextStream in(&file);
    QRegularExpression timeTagRegex(R"(\[(\d{2}):(\d{2})(?:\.(\d{2,3}))?\])");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatchIterator i = timeTagRegex.globalMatch(line);

        int lastIndex = 0;
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            int minutes = match.captured(1).toInt();
            int seconds = match.captured(2).toInt();
            int milliseconds = match.captured(3).toInt();
            if (match.captured(3).length() == 2) {
                milliseconds *= 10;
            }

            qint64 totalMs = minutes * 60000 + seconds * 1000 + milliseconds;

            lastIndex = match.capturedEnd();
            QString text = line.mid(lastIndex).trimmed();

            lyrics.append({ totalMs, text });
        }
    }

    return lyrics;
}

QString MainWindow::GetTimeFormat(qint64 ms)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(ms);

    return time.hour() > 0 ? time.toString("h:mm:ss") : time.toString("m:ss");
}

void MainWindow::LoadQss(const QString& qssFilename)
{
    QFile qssFile(qssFilename);
    if (!qssFile.open(QIODevice::ReadOnly))
    {
        return;
    }
    QString strQss = qssFile.readAll();
    qApp->setStyleSheet(strQss);
    qssFile.close();
}

void MainWindow::on_pushButton_play_clicked()
{
    QMediaPlayer::PlaybackState status = m_pPlayer->playbackState();
    if (status != QMediaPlayer::PlayingState)
    {
        m_pPlayer->play();
    }
    else
    {
        m_pPlayer->pause();
    }
}

