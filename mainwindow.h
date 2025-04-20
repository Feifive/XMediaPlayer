#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QMediaPlayer;

struct LyricLine 
{
    qint64  timeMs; //  ±º‰¥¡£®∫¡√Î£©
    QString text;  // ∏Ë¥ ƒ⁄»›
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

private:
    QList<LyricLine> ParseLrcFile(const QString& filePath);
    QString GetTimeFormat(qint64 ms);
    void LoadQss(const QString& qssFilename);

private:
    Ui::MainWindow *ui;
    QMediaPlayer* m_pPlayer;
    QList<LyricLine> m_lyrics;
    int m_iCurrentLyricLine;
    QPixmap m_currentCoverPixmap;
};
#endif // MAINWINDOW_H
