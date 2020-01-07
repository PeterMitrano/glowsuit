#pragma once

#include <string>
#include <map>
#include <optional>

#include <QCloseEvent>
#include <QMediaPlayer>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QWidget>

#include <../src/ui_mainwidget.h>

#include <midi/midi_input.h>
#include <midi/midi_file_player.h>
#include <serial/serial.h>
#include <visualizer.h>

std::optional<json> load_suit_description();

QString select_music_file(QWidget *parent);

QString select_midi_file(QWidget *parent);

class MainWidget : public QWidget
{
Q_OBJECT

public:
    virtual ~MainWidget();

    explicit MainWidget(QWidget *parent = nullptr);

    void save_settings();

    void restore_settings();

    void closeEvent(QCloseEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void set_state(QMediaPlayer::State state);

    QMediaPlayer::State state() const;

    void update_duration_info(qint64 currentInfo);

    void handle_cursor(QMediaPlayer::MediaStatus status);

    void blink_midi_indicator();

public slots:

    void event_count_changed(int event_count);

    void num_tracks_changed(int num_tracks);

    void update_serial_port_list();

    void live_midi_changed(int state);

    void play_pause_clicked();

    void midi_file_button_clicked();

    void music_file_button_clicked();

    void xbee_port_changed(int index);

    void seek(int milliseconds);

    void duration_changed(qint64 duration);

    void position_changed(qint64 progress);

    void status_changed(QMediaPlayer::MediaStatus status);

    void display_error_message();

    void any_event();

    void all_on_clicked();

    void all_off_clicked();

signals:

    void midi_file_changed(QString midi_filename);

    void play();

    void pause();

    void stop();

    void gui_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

public:
    Visualizer *visualizer{nullptr};
    LiveMidiWorker *live_midi_worker{nullptr};
    QString music_filename;
    QString midi_filename;
    QThread live_midi_thread;
    smf::MidiFile midifile;
    std::map<unsigned int, State> states;
    // TODO: better than raw pointer? problem with optional is serial::Serial is not copyable
    serial::Serial *xbee_serial{nullptr};
    QSettings *settings;


private:
    Ui_MainWidget ui;

    QMediaPlayer::State player_state = QMediaPlayer::StoppedState;
    QMediaPlayer *music_player{nullptr};
    QThread midi_player_thread;
    MidiFilePlayer *midi_file_player{nullptr};

    qint64 song_duration_ms{0};
    QTimer *timer{nullptr};
    int num_channels{0};
    bool controls_hidden{false};
    QLayoutItem *controls_layout_item{nullptr};
    QString previously_selected_port_name;
};
