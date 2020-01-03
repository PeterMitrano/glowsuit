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

class MidiFilePlayer : public QObject
{
    Q_OBJECT

public:

    MidiFilePlayer(QWidget* parent = nullptr);

    virtual ~MidiFilePlayer();

    serial::Serial* xbee_serial{ nullptr };

    void start_thread();

    void parse_midifile();

    void emit_to_visualizer(State const& state);

signals:
    void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

public slots:
    void midi_file_changed(QString midi_filename);

    void position_changed(qint64 time_ms);

    void play();

    void pause();

    void stop();

    void octave_spinbox_changed(int value);

private:
    int octave_offset;
    bool playing{ false };
    smf::MidiFile midifile;
    std::vector<std::pair<unsigned int, State>> states_vector;
    QMutex mutex;
    bool killed{ false };
    std::thread thread;
};
