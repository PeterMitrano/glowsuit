#pragma once

#include <thread>
#include <optional>

#include <QMutex>
#include <QObject>
#include <QWidget>

#include <byteset.h>
#include <serial/serial.h>
#include <midi/MidiFile.h>
#include <midi/midi_common.h>

using OnsetStatePair = std::pair<int, State>;
using OnsetStateVec = std::vector<OnsetStatePair>;
using TimePoint = std::chrono::high_resolution_clock::time_point;

qint64 current_song_time(TimePoint clock_reference, qint64 song_time_reference);

class MidiFilePlayer : public QObject
{
Q_OBJECT

public:

    MidiFilePlayer(QWidget *parent = nullptr);

    virtual ~MidiFilePlayer();

    serial::Serial *xbee_serial{nullptr};

    void start_thread();

    void parse_track();

    void emit_to_visualizer(State const &state);

signals:

    void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void track_range_changed(int min, int max);

    void event_count_changed(int count);

public slots:

    void midi_file_changed(QString midi_filename);

    void seek(int seconds);

    void play();

    void pause();

    void stop();

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
    int track_number{0};
};
