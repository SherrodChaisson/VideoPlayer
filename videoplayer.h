#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QSqlTableModel>
#include <QSlider>
#include <QPushButton>

namespace Ui {
class VideoPlayer;
}

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

private:

    void initDatabase();

    void initVideo();

    void initVolume();

    void initButtons();

    void initVideoTableView();

    void initVideoList();

    void initTimer();

    // Database
    bool add2Database(const QString &name, const QString &path);

    // Video
    void playVideo();

    // Helper
    QString ms2TimeFormat(qint64 milliseconds);

    void setButtonsStyle(QPushButton* btn);

private slots:

    // Video
    void setVideoPosAccording2Slider(int value);

    void setVideoSliderAccording2VideoPos();

    void setSliderMaximum(qint64 duration);

    void playVideoInTable(const QModelIndex &index);

    void setVideoPlayerMode(QMediaPlayer::MediaStatus status);

    // Volume
    void on_volume_close_push_button_clicked();

    void on_volume_add_push_button_clicked();

    void on_volume_sub_push_button_clicked();

    // Buttons
    void setPlayButtonIcon(QMediaPlayer::State state);

    void on_exit_push_button_clicked();

    void on_video_speed_combo_box_currentIndexChanged(int index);

    void on_video_autoplay_radio_button_clicked();

    void on_prior_push_button_clicked();

    void on_next_push_button_clicked();

    void on_file_push_button_clicked();

    void on_play_push_button_clicked();

    // Video list
    void updateVideoTableViewSelected();

private:
    Ui::VideoPlayer *ui;
    QMediaPlayer *media_player;
    QMediaPlaylist *media_playlist;
    QSqlDatabase database;

    QSqlTableModel *table_model;
    bool video_autoplay_bool = false;
    QTimer *play_timer;
    QTimer *video_slider_timer;
    int video_curr_list_index = 0;
};

#endif // VIDEOPLAYER_H
