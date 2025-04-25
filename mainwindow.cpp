#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "3rdparty/taglib/include/taglib_export.h"
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QAudioOutput>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDir>
#include <QDebug>
#include <qfilesystemwatcher.h>
#include "3rdparty/taglib/include/mpegfile.h"
#include "3rdparty/taglib/include/id3v2tag.h"
#include "3rdparty/taglib/include/attachedpictureframe.h"
#include "3rdparty/taglib/include/fileref.h"
#include "3rdparty/taglib/include/tag.h"
#include "3rdparty/taglib/include/uniquefileidentifierframe.h"
#include "3rdparty/taglib/include/unsynchronizedlyricsframe.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    m_iCurrentLyricLine(0),
    m_iCurrentSongIndex(0)
{
    ui->setupUi(this);
    LoadQss(QApplication::applicationDirPath() + "/styles/style.qss");
    QFileSystemWatcher* pWatcher = new QFileSystemWatcher(QStringList() << QApplication::applicationDirPath() + "/styles/style.qss", this);
    connect(pWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::LoadQss);

    ui->label_cover->setMinimumSize(QSize(100, 100));

    m_pPlayer = new QMediaPlayer;
    QAudioOutput* audioOutput = new QAudioOutput;
    audioOutput->setVolume(100);
    m_pPlayer->setAudioOutput(audioOutput);
    connect(m_pPlayer, &QMediaPlayer::positionChanged, ui->playSlider, [this](qint64 position)
    {
        if (m_iCurrentLyricLine <= m_lyrics.size() - 1)
        {
            if (position > m_lyrics[m_iCurrentLyricLine].timeMs && m_lyrics[m_iCurrentLyricLine].timeMs != -1)
            {
                ui->listWidget_lyricsView->setCurrentRow(m_iCurrentLyricLine);
                ui->listWidget_lyricsView->scrollToItem(ui->listWidget_lyricsView->currentItem());
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

            if (!m_currentCoverPixmap.isNull())
            {
                ui->label_cover->setPixmap(m_currentCoverPixmap.scaled(ui->label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
    });

    SetPlayList("C:/Users/Ze/Music");

    if (!m_playList.isEmpty())
    {
        m_pPlayer->setSource(QUrl::fromLocalFile(m_playList.first().absoluteFilePath()));
        LoadLyric(m_playList.first());
    }

    QImage cover;

    bool bLoad = LoadCover("C:/Users/Ze/Music/王菲-如愿.mp3", cover);

    if (bLoad && !cover.isNull())
    {
        m_currentCoverPixmap = QPixmap::fromImage(cover);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    if (!m_currentCoverPixmap.isNull())
    {
        QSize size = ui->label_cover->size();
        ui->label_cover->setPixmap(m_currentCoverPixmap.scaled(ui->label_cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    return QMainWindow::resizeEvent(event);
}

QList<LyricLine> MainWindow::ParseLrcFile(const QString& filePath)
{
    QList<LyricLine> lyrics;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        qWarning() << "Failed to open file:" << filePath;
        return lyrics;
    }

    QTextStream in(&file);
    QRegularExpression timeTagRegex(R"(\[(\d{2}):(\d{2})(?:\.(\d{2,3}))?\])");

    while (!in.atEnd()) 
    {
        QString line = in.readLine();
        QRegularExpressionMatchIterator i = timeTagRegex.globalMatch(line);

        int lastIndex = 0;
        while (i.hasNext()) 
        {
            QRegularExpressionMatch match = i.next();
            int minutes = match.captured(1).toInt();
            int seconds = match.captured(2).toInt();
            int milliseconds = match.captured(3).toInt();
            if (match.captured(3).length() == 2) 
            {
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

void MainWindow::SetPlayList(const QString& folderPath)
{
    // 定义常见的音频文件扩展名
    QStringList audioExtensions = 
    {
        "*.mp3", "*.wav", "*.ogg", "*.flac", "*.aac",
        "*.wma", "*.m4a", "*.aiff", "*.opus"
    };

    QDir directory(folderPath);
    m_playList = directory.entryInfoList(audioExtensions, QDir::Files | QDir::NoDotAndDotDot);
}

void MainWindow::LoadLyric(const QFileInfo& info)
{
    m_iCurrentLyricLine = 0;
    m_lyrics.clear();
    ui->listWidget_lyricsView->clear();
    QString strLcrFilename = info.absolutePath() + "/" + info.baseName() + ".lrc";
    if (QFile::exists(strLcrFilename))
    {
        m_lyrics = ParseLrcFile(strLcrFilename);
    }
    else
    {
        GetEmbeddedLyrics(info.absoluteFilePath());
    }

    if (m_lyrics.size() > 0)
    {
        AddLyricsToLyricsView(m_lyrics);
    }
}

bool MainWindow::GetEmbeddedLyrics(const QString& fileName)
{
    if (!fileName.endsWith("mp3"))
    {
        return false;
    }

    TagLib::MPEG::File mpegFile(fileName.toStdWString().c_str());
    if (!mpegFile.isValid() || !mpegFile.hasID3v2Tag())
    {
        return false;
    }

    // 获取所有歌词帧
    TagLib::ID3v2::FrameList frames = mpegFile.ID3v2Tag()->frameListMap()["USLT"];
    if (frames.isEmpty())
    {
        return false;
    }

    // 取第一个USLT帧内容
    TagLib::ID3v2::UnsynchronizedLyricsFrame* lyricsFrame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(frames.front());
    if (!lyricsFrame)
    {
        return false;
    }

    // 处理编码（中文可能需要转换）
    QString strLyrics = QString::fromStdString(lyricsFrame->text().toCString(true));
    QStringList listLyrics = strLyrics.split("\n");
    for(const auto& lyric : listLyrics)
    {
        m_lyrics.append({ -1, lyric });
    }

    return true;
}

bool MainWindow::LoadCover(const QString& fileName, QImage& image)
{
    TagLib::MPEG::File mpegFile(fileName.toStdWString().c_str());
    if (!mpegFile.isValid() || !mpegFile.hasID3v2Tag())
    {
        return false;
    }

    // 2. 获取所有 APIC（专辑封面）帧
    TagLib::ID3v2::FrameList frames = mpegFile.ID3v2Tag()->frameListMap()["APIC"];
    if (frames.isEmpty()) 
    {
        return false;
    }

    // 3. 查找第一个封面图片帧
    auto* coverFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
    if (!coverFrame) 
    {
        return false;
    }

    // 4. 将图片数据转换为 QImage
    QByteArray imageData(coverFrame->picture().data(), coverFrame->picture().size());
    if (!image.loadFromData(imageData))
    {
        return false;
    }

    return true;
}

void MainWindow::AddLyricsToLyricsView(const QList<LyricLine>& lyrics)
{
    for (const auto& lyric : lyrics)
    {
        QListWidgetItem* pItem = new QListWidgetItem(lyric.text);
        pItem->setTextAlignment(Qt::AlignCenter);
        ui->listWidget_lyricsView->addItem(pItem);
    }
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


void MainWindow::on_pushButton_previous_clicked()
{
    if (m_playList.isEmpty())
    {
        return;
    }

    --m_iCurrentSongIndex;
    if (m_iCurrentSongIndex < 0)
    {
        m_iCurrentSongIndex = m_playList.size() - 1;
    }

    m_pPlayer->stop();
    m_pPlayer->setSource(QUrl::fromLocalFile(m_playList[m_iCurrentSongIndex].absoluteFilePath()));
    LoadLyric(m_playList[m_iCurrentSongIndex]);
    m_pPlayer->play();
}

void MainWindow::on_pushButton_next_clicked()
{
    if (m_playList.isEmpty())
    {
        return;
    }

    ++m_iCurrentSongIndex;
    if (m_iCurrentSongIndex >= m_playList.size())
    {
        m_iCurrentSongIndex = 0;
    }

    m_pPlayer->stop();
    m_pPlayer->setSource(QUrl::fromLocalFile(m_playList[m_iCurrentSongIndex].absoluteFilePath()));
    LoadLyric(m_playList[m_iCurrentSongIndex]);
    m_pPlayer->play();
}

