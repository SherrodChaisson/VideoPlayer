#include "videoplayer.h"
#include "ui_videoplayer.h"
#include "myslider.h"

#include <QFileDialog>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <QTimer>
#include <QTableView>
#include <QWidget>

VideoPlayer::VideoPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoPlayer)
{
    // Create
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/images/jlu.png"));

    media_player = new QMediaPlayer(this);
    media_playlist = new QMediaPlaylist;
    play_timer = new QTimer(this);
    video_slider_timer = new QTimer(this);


    initTimer();
    initDatabase();
    initVideo();
    initVolume();
    initButtons();
    initVideoTableView();
    initVideoList();

    updateVideoTableViewSelected();
}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

void VideoPlayer::initDatabase()
{
    // Set datebase
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("test11.db");
    if (database.open()) {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS videos (name TEXT, path TEXT)");
    }else{
        QMessageBox::warning(0, QObject::tr("Database Error"), database.lastError().text());
    }
}

void VideoPlayer::initVideo()
{
    // Set video
    media_player->setVideoOutput(ui->video_widget);
    media_player->setPlaylist(media_playlist);
    ui->video_horizontal_slider->setValue(0);
    QFont font;
    font.setFamily("Arial"); // 设置字体族
    font.setPointSize(10);   // 设置字体大小
    font.setBold(true);      // 设置字体为粗体
    font.setItalic(false);   // 设置字体不为斜体
    font.setUnderline(false); // 设置字体无下划线

    ui->video_name_label->setFont(font);
    ui->playlist_label->setFont(font);

    // Video slider's connection
    connect(ui->video_horizontal_slider,&QSlider::valueChanged,this,&VideoPlayer::setVideoPosAccording2Slider);
    connect(media_player,&QMediaPlayer::durationChanged,this,&VideoPlayer::setSliderMaximum);

    // Video
    connect(media_player,&QMediaPlayer::mediaStatusChanged,this,&VideoPlayer::setVideoPlayerMode);
    connect(ui->video_table_view,&QTableView::clicked,this,&VideoPlayer::playVideoInTable);

}

void VideoPlayer::initVolume()
{
    // Set volume
    media_player->setVolume(50);
}

void VideoPlayer::initButtons()
{
    // Buttons' connection
    connect(media_player,&QMediaPlayer::stateChanged,this,&VideoPlayer::setPlayButtonIcon);
    ui->play_push_button->setIcon(QIcon(":/images/pause.png"));

    this->setButtonsStyle(ui->play_push_button);
    this->setButtonsStyle(ui->next_push_button);
    this->setButtonsStyle(ui->prior_push_button);
    this->setButtonsStyle(ui->file_push_button);
    this->setButtonsStyle(ui->volume_close_push_button);
    this->setButtonsStyle(ui->volume_add_push_button);
    this->setButtonsStyle(ui->volume_sub_push_button);
    this->setButtonsStyle(ui->exit_push_button);

    // Speed combox
    ui->video_speed_combo_box->setCurrentIndex(3);
}

void VideoPlayer::initVideoTableView()
{
    // Create database table model
    table_model = new QSqlTableModel(this);
    table_model->setTable("videos");
    // Get data from table
    table_model->select();

    ui->video_table_view->verticalHeader()->hide();
    ui->video_table_view->horizontalHeader()->hide();

    ui->video_table_view->setModel(table_model);

    // 隐藏路径所在的列
    ui->video_table_view->hideColumn(1); // 1是路径所在的列索引，从0开始计数
    ui->video_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//让所有的列均匀地填充整个QTableView，这样所有的列宽将会根据QTableView的大小进行自动调整，以使所有的列都可见，并且填满QTableView的宽度。
    ui->video_table_view->setSelectionMode(QAbstractItemView::SingleSelection);//表示只能选择单个项目（单行或单列）
    while(table_model->canFetchMore()){
        table_model->fetchMore();
    }
}

void VideoPlayer::initVideoList()
{
    // Set playlist
    for (int row = 0; row < table_model->rowCount(); ++row) {
                QModelIndex index = table_model->index(row, table_model->fieldIndex("path"));
                QString path = table_model->data(index).toString();
                media_playlist->addMedia(QUrl(path));
    }
}

void VideoPlayer::initTimer()
{
    play_timer->setSingleShot(true);
    play_timer->setInterval(500);
    connect(play_timer, &QTimer::timeout, this, &VideoPlayer::playVideo);

    video_slider_timer->setInterval(50);
    connect(video_slider_timer, &QTimer::timeout, this, &VideoPlayer::setVideoSliderAccording2VideoPos);
    video_slider_timer->start();
}

void VideoPlayer::setPlayButtonIcon(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::PlayingState) {
        ui->play_push_button->setIcon(QIcon(":/images/play.png"));
    }
    else {
        ui->play_push_button->setIcon(QIcon(":/images/pause.png"));
    }
}

void VideoPlayer::setVideoPosAccording2Slider(int value)
{

    if (media_player->duration() > 0 && ui->video_horizontal_slider->isSliderSelected()) {
         media_player->setPosition(value);
    }
}

void VideoPlayer::setVideoSliderAccording2VideoPos()
{
    if (media_player->duration() > 0) {
        auto pos = media_player->position();
        ui->video_horizontal_slider->setValue(pos);
        ui->video_curr_length_label->setText(this->ms2TimeFormat(pos)+"/");
    }
}

void VideoPlayer::setSliderMaximum(qint64 duration)
{
    if(duration > 0){
        ui->video_horizontal_slider->setMaximum(duration);
        ui->video_length_label->setText(this->ms2TimeFormat(duration));
    }

}

void VideoPlayer::playVideoInTable(const QModelIndex &index)
{
    int row = index.row();
    int col = index.column();

    int pathColumn = table_model->fieldIndex("path");

    // 如果点击的是视频名字所在的列，则播放该视频
    if (col == 0)
    {
        QString videoPath = table_model->data(table_model->index(row, pathColumn)).toString();
        video_curr_list_index = row;
        updateVideoTableViewSelected();
        media_playlist->setCurrentIndex(video_curr_list_index);
        playVideo();
    }
}

void VideoPlayer::setVideoPlayerMode(QMediaPlayer::MediaStatus status)
{
    if(status ==  QMediaPlayer::EndOfMedia){
        switch (ui->video_play_mode_combo_box->currentIndex()) {
            case 0:
                media_playlist->setPlaybackMode(QMediaPlaylist::Loop);
                this->on_next_push_button_clicked();
                break;
            case 1:
                media_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
                break;
            default:
                break;
        }
        media_player->pause();
        if(this->video_autoplay_bool){
            play_timer->start();
        }
    }
}

void VideoPlayer::playVideo()
{
    ui->video_horizontal_slider->setValue(0);
    media_player->play();
    updateVideoTableViewSelected();
    ui->video_name_label->setText(ui->video_table_view->model()->index(this->video_curr_list_index, 0).data().toString());
}

QString VideoPlayer::ms2TimeFormat(qint64 milliseconds)
{
    int totalSeconds = milliseconds / 1000;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    QString timeFormat;
    timeFormat.sprintf("%02d:%02d:%02d", hours, minutes, seconds);

    return timeFormat;
}

inline void VideoPlayer::setButtonsStyle(QPushButton *btn)
{
    btn->setStyleSheet("QPushButton {"
                                        "  background-color: transparent;"
                                        "  border: none;"
                                        "  color: white;" // 设置文本颜色
                                        "}"
                                        "QPushButton:hover {"
                                        "  background-color: rgba(255, 255, 255, 50);" // 鼠标悬停时的半透明背景
                                        "}"
                                        "QPushButton:pressed {"
                                        "  background-color: transparent;"
                                        "}");

}

// Return value will tell if the video has been in the play list
bool VideoPlayer::add2Database(const QString &name, const QString &path)
{
    QSqlQuery query;
    query.prepare("SELECT name, path FROM videos WHERE name = :name");
    query.bindValue(":name", name);
    query.exec();

    if (query.next()) // 如果有重复的文件名
    {
        QString existingPath = query.value("path").toString();

        if (existingPath != path)
        {
            // 删除之前的文件名
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM videos WHERE name = :name AND path = :path");
            deleteQuery.bindValue(":name", name);
            deleteQuery.bindValue(":path", existingPath);
            deleteQuery.exec();
        }
        else
        {
            // 如果路径相同，说明数据完全相同，不做任何操作
            return true;
        }
    }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO videos (name, path) VALUES (:name, :path)");
    insertQuery.bindValue(":name", name);
    insertQuery.bindValue(":path", path);
    insertQuery.exec();

    table_model->select();
    return false;

}

// Exit
void VideoPlayer::on_exit_push_button_clicked()
{
    this->close();
}

// Change the video's speed
void VideoPlayer::on_video_speed_combo_box_currentIndexChanged(int index)
{
    switch(index){
        case 0:
            media_player->setPlaybackRate(2);
            break;
        case 1:
            media_player->setPlaybackRate(1.5);
            break;
        case 2:
            media_player->setPlaybackRate(1.25);
            break;
        case 3:
            media_player->setPlaybackRate(1);
            break;
        case 4:
            media_player->setPlaybackRate(0.75);
            break;
        case 5:
            media_player->setPlaybackRate(0.5);
            break;
        default:
            break;
    }
}

void VideoPlayer::on_video_autoplay_radio_button_clicked()
{
    video_autoplay_bool = ui->video_autoplay_radio_button->isChecked();
}

void VideoPlayer::on_prior_push_button_clicked()
{
    --video_curr_list_index;
    video_curr_list_index = (video_curr_list_index < 0) ? (media_playlist->mediaCount() - 1)  : video_curr_list_index;
    media_playlist->setCurrentIndex(video_curr_list_index);
    updateVideoTableViewSelected();
}

void VideoPlayer::on_next_push_button_clicked()
{
    ++video_curr_list_index;
    video_curr_list_index = (video_curr_list_index >= media_playlist->mediaCount()) ? 0 : video_curr_list_index;
    media_playlist->setCurrentIndex(video_curr_list_index);
    updateVideoTableViewSelected();
}

void VideoPlayer::updateVideoTableViewSelected()
{
    QItemSelectionModel *selectionModel = ui->video_table_view->selectionModel();
    selectionModel->clearSelection();

    // 选中整行
    QModelIndex index = ui->video_table_view->model()->index(this->video_curr_list_index, 0);// 获取行的第一个单元格的索引
    selectionModel->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    // 如果需要，可以更新视图以显示选中状态
    ui->video_table_view->update();
}

// Convert button state
void VideoPlayer::on_play_push_button_clicked()
{
    if(QMediaPlayer::PlayingState == media_player->state()){
        media_player->pause();
    }
    else{
        media_player->play();
        updateVideoTableViewSelected();
        ui->video_name_label->setText(ui->video_table_view->model()->index(this->video_curr_list_index, 0).data().toString());
    }
}

// Add new file
void VideoPlayer::on_file_push_button_clicked()
{
    bool prior_video_autoplay_bool = this->video_autoplay_bool;
    this->video_autoplay_bool = false;

    media_player->stop();

    QString file_path = QFileDialog::
            getOpenFileName(this, tr("请选择要播放的文件"), QString(), tr("Media Files (*.mp4 *.mp3 *.avi *.mkv);;All Files (*)"));

    if (!file_path.isEmpty()) {
        media_playlist->clear();

        QString fileName = QFileInfo(file_path).fileName();
        auto flag = add2Database(fileName, file_path);
        ui->video_table_view->scrollToBottom();

        for (int row = 0; row < table_model->rowCount(); ++row) {
                    QModelIndex index = table_model->index(row, table_model->fieldIndex("path"));
                    QString path = table_model->data(index).toString();
                    media_playlist->addMedia(QUrl(path));
                    if(flag){
//                        qDebug()<<index.data().toString();
                        if(index.data().toString() == file_path){
                            video_curr_list_index = index.row();
//                            qDebug()<<index.data().toString();
                        }
                    }
        }
        // If new video
        if(!flag){
            video_curr_list_index = media_playlist->mediaCount() - 1;
        }
        media_playlist->setCurrentIndex(video_curr_list_index);
        updateVideoTableViewSelected();

        playVideo();
    }
    this->video_autoplay_bool = prior_video_autoplay_bool;
}

void VideoPlayer::on_volume_close_push_button_clicked()
{
    media_player->setVolume(0);
}

void VideoPlayer::on_volume_add_push_button_clicked()
{
    media_player->setVolume(media_player->volume()+10);
}

void VideoPlayer::on_volume_sub_push_button_clicked()
{
    media_player->setVolume(media_player->volume()-10);
}
