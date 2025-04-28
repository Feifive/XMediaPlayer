#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfoList>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QMediaPlayer;

struct LyricLine 
{
    qint64  timeMs;
    QString text;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void on_pushButton_play_clicked();
    void on_pushButton_previous_clicked();
    void on_pushButton_next_clicked();

private:
    QList<LyricLine> ParseLrcFile(const QString& filePath);
    QString GetTimeFormat(qint64 ms);
    void LoadQss(const QString& qssFilename);
    void SetPlayList(const QString& folderPath);
    void LoadLyric(const QFileInfo& info);
    bool GetEmbeddedLyrics(const QString& filename, QList<LyricLine>& lyrics);
    bool GetEmbeddedCover(const QString& filename, QImage& image);
    bool LoadCover(const QString& filename);
    void AddLyricsToLyricsView(const QList<LyricLine>& lyrics);

private:
    Ui::MainWindow *ui;
    QMediaPlayer* m_pPlayer;
    QList<LyricLine> m_lyrics;
    QFileInfoList m_playList;
    int m_iCurrentLyricLine;
    int m_iCurrentSongIndex;
    QPixmap m_currentCoverPixmap;
};
#endif // MAINWINDOW_H
