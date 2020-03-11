#pragma once

#include <string>
#include <thread>
#include <map>
#include <optional>

#include <QCheckBox>
#include <QCloseEvent>
#include <QMediaPlayer>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QWidget>

#include <../src/ui_mainwidget.h>

#include <midi/live_midi_input.h>
#include <serial/serial.h>
#include <visualizer.h>

std::optional<json> load_suit_description();

QString select_music_file(QWidget *parent);

class MainWidget : public QWidget
{
Q_OBJECT

public:
    ~MainWidget() override;

    explicit MainWidget(QWidget *parent = nullptr);

    void save_settings();

    void restore_settings();

    void closeEvent(QCloseEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    [[nodiscard]] QMediaPlayer::State state() const;

    void handle_cursor(QMediaPlayer::MediaStatus status);

    void blink_midi_indicator();

    void sendTime(long dt_ms);

public slots:

    void num_suits_changed(int value);

    void update_serial_port_list();

    void live_midi_changed(int state);

    void music_file_button_clicked();

    void xbee_port_changed(int index);

    void status_changed(QMediaPlayer::MediaStatus status);

    void display_error_message();

    void any_event();

    void all_on_clicked();

    void all_off_clicked();

    void start_clicked();

    void abort_clicked();

signals:

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
    std::thread sync_xbees_thread;

    qint64 song_duration_ms{0};
    uint8_t num_suits{0u};
    bool aborted{false};
    QTimer *timer{nullptr};
    int num_channels{0};
    bool controls_hidden{false};
    QLayoutItem *controls_layout_item{nullptr};
    QString previously_selected_port_name;
    std::vector<QCheckBox *> suit_status_checkboxes;

};
