#pragma once

#include <QObject>

#include <serial/serial.h>
#include <midi/MidiFile.h>
#include <midi/midi_common.h>
#include <alsa/asoundlib.h>     /* Interface to the ALSA system */

class LiveMidiWorker : public QObject
{
Q_OBJECT

public:
    LiveMidiWorker(size_t num_channels, QWidget *parent_widget);

    int start_midi();

    serial::Serial *xbee_serial{nullptr};

public slots:

    void listen_for_midi();

    void octave_spinbox_changed(int value);

signals:

    void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void my_finished();

private:
    QWidget *parent_widget;
    size_t num_channels;
    int octave_offset{0};

    State current_state;
};
