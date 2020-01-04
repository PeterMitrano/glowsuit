#include <QMessageBox>
#include <QThread>

#include <midi/linux/midi_input_linux.h>
#include <sstream>
#include <common.h>

static snd_seq_t *seq_handle;
static int in_port;

// FIXME: proper error handling
#define CHK(stmt, msg) if((stmt) < 0) {puts("ERROR: "#msg); exit(1);}

unsigned int alsa_seq_type_to_midi_command(snd_seq_event_type_t type);

unsigned int alsa_seq_type_to_midi_command(snd_seq_event_type_t type)
{
    switch (type)
    {
        case SND_SEQ_EVENT_NOTEOFF:
            return 128;
        case SND_SEQ_EVENT_NOTEON:
            return 144;
        default:
            return 0;
    }
}

void midi_open()
{
    CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0),
        "Could not open sequencer");

    CHK(snd_seq_set_client_name(seq_handle, "glowsuit"),
        "Could not set client name");
    CHK(in_port = snd_seq_create_simple_port(seq_handle, "in",
                                             SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                             SND_SEQ_PORT_TYPE_APPLICATION),
        "Could not open port");
    snd_seq_nonblock(seq_handle, 1);
}

LiveMidiWorker::LiveMidiWorker(size_t num_channels, QWidget *parent_widget)
        : QObject(nullptr),
          parent_widget(parent_widget),
          num_channels(num_channels)
{}

void LiveMidiWorker::listen_for_midi()
{
    auto return_code = start_midi();
    if (return_code == -1)
    {
        //TODO handle failures here
    }
    emit my_finished();
}

int LiveMidiWorker::start_midi()
{
    midi_open();
    while (true)
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            break;
        }

        snd_seq_event_t *ev = nullptr;
        auto const result = snd_seq_event_input(seq_handle, &ev);
        if (result == -EAGAIN)
        {
            continue;
        }

        if (ev->type != SND_SEQ_EVENT_NOTEON && ev->type != SND_SEQ_EVENT_NOTEOFF)
        {
            continue;
        }

        auto const command = alsa_seq_type_to_midi_command(ev->type);
        auto const pitch = static_cast<int>(ev->data.note.note);
        int const bit_idx = pitch - midi_note_offset + 12 * octave_offset;

        if (bit_idx < 0 || bit_idx >= 6 * 8)
        {
            continue;
        }

        auto const channel_number = bit_idx % num_channels;
        auto const suit_idx = static_cast<unsigned int>(bit_idx / num_channels);
        auto const suit_number = suit_idx + 1;

        // send a message to the visualizer
        emit midi_event(suit_number, command, channel_number);

        // update the current state and send the data
        if (command == 128)
        {
            current_state.set(bit_idx, false);
        } else if (command == 144)
        {
            current_state.set(bit_idx, true);
        } else
        {
            // just skip if it's not NOTE ON or NOTE OFF
            continue;
        }

        // transmit to suits
        if (xbee_serial != nullptr)
        {
            xbee_serial->write(current_state.data.data(), message_size);
        }
    }
}

void LiveMidiWorker::octave_spinbox_changed(int value)
{
    octave_offset = value;
}
