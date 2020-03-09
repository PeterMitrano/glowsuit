#pragma once

#include <thread>
#include <optional>

#include <QMutex>
#include <QObject>
#include <QMediaPlayer>
#include <QWidget>

#include <byteset.h>
#include <serial/serial.h>
#include <midi/MidiFile.h>
#include <common.h>

struct OnsetState
{
    int onset_ms;
    State state;

    OnsetState(int onset_ms, State state) : onset_ms(onset_ms), state(state) {}
};

using OnsetStateVec = std::vector<OnsetState>;
using TimePoint = std::chrono::high_resolution_clock::time_point;

qint64 current_song_time(TimePoint clock_reference, qint64 song_time_reference);

constexpr auto const WindowSize{100u};

struct Window
{
    std::array<State, WindowSize> states;
    qint64 current_song_time{};
};

class MidiFilePlayer : public QObject
{
Q_OBJECT

public:

    MidiFilePlayer(QWidget *parent = nullptr);

    virtual ~MidiFilePlayer();

    serial::Serial *xbee_serial{nullptr};

    void start_thread();

    void parse_track();

    void emit_to_visualizer(Window const &states_window);

    void emit_to_visualizer(State const &state);

    void emit_to_xbee(Window const &states_window);

    void emit_to_xbee(State const &state);

signals:

    void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void num_tracks_changed(int num_tracks);

    void event_count_changed(int count);

public slots:

    void midi_file_changed(QString midi_filename);

    void seek(int time_ms);

    void play();

    void pause();

    void stop();

    void status_changed(QMediaPlayer::MediaStatus status);

    void track_changed(int value);

    void octave_spinbox_changed(int value);

private:
    int octave_offset{0};
    bool playing{false};
    smf::MidiFile midifile;
    OnsetStateVec states_vector;
    QMutex mutex;
    bool killed{false};
    std::thread thread;
    qint64 latest_song_time_reference{0};
    qint64 current_song_time_ms{0};
    size_t current_state_idx{0ul};
    TimePoint latest_clock_reference;
    bool use_visualizer{true};
    int track_number{0};
};
