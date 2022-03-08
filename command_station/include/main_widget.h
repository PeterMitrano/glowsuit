#pragma once

#include <../src/ui_mainwidget.h>
#include <live_midi_input.h>
#include <serial/serial.h>
#include <suit_common.h>
#include <visualizer.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QMediaPlayer>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QWidget>
#include <map>
#include <optional>
#include <string>
#include <thread>

std::optional<json> load_suit_description();

QString select_music_file(QWidget *parent);

class MainWidget : public QWidget {
  Q_OBJECT

 public:
  ~MainWidget() override;

  explicit MainWidget(QWidget *parent = nullptr);

  void save_settings();

  void restore_settings();

  void closeEvent(QCloseEvent *event) override;

  void keyReleaseEvent(QKeyEvent *event) override;

  void set_state(QMediaPlayer::State state);

  void update_duration_info(qint64 currentInfo);

  [[nodiscard]] QMediaPlayer::State state() const;

  void handle_cursor(QMediaPlayer::MediaStatus status);

  void start_with_countdown();

  void blink_midi_indicator();

  void sendTime(qint64 song_time_ms);

 public slots:

  void play_pause_clicked();

  void all_off_clicked();

  void update_serial_port_list();

  void live_midi_changed(int state);

  void music_file_button_clicked();

  void xbee_port_changed(int index);

  void seek(int milliseconds);

  void duration_changed(qint64 duration);

  void position_changed(qint64 progress);

  void status_changed(QMediaPlayer::MediaStatus status);

  void display_error_message();

  void any_event();

 signals:

  void play();

  void pause();

  void stop();

  void gui_midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

  void software_suits_xbee_write(Data command);

 public:
  Visualizer *visualizer{nullptr};
  LiveMidiWorker *live_midi_worker{nullptr};
  std::vector<QThread *> suit_threads;
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
  std::thread startup_sync_thread;

  qint64 song_duration_ms{0};
  QTimer *timer{nullptr};
  int num_channels{0};
  bool controls_hidden{false};
  QLayoutItem *controls_layout_item{nullptr};
  QString previously_selected_port_name;

  long start_ms_{0L};
  constexpr static long const start_delay_ms_{5000L};
  bool choreo_started_once{false};
};
