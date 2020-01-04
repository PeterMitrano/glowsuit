#include <QMessageBox>
#include <QThread>

#include <midi/linux/midi_input_linux.h>
#include <sstream>
#include <common.h>

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

snd_seq_t *LiveMidiWorker::midi_open()
{
    snd_seq_t *seq_handle{nullptr};
    int result;
    result = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
    if (result < 0)
    {
        QMessageBox::warning(parent_widget, tr("ALSA Error"), tr("failed to open midi sequencer"));
    }

    result = snd_seq_set_client_name(seq_handle, "glowsuit");
    if (result < 0)
    {
        QMessageBox::warning(parent_widget, tr("ALSA Error"), tr("failed to name midi sequencer"));
    }
    result = snd_seq_create_simple_port(seq_handle, "in",
                                        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                        SND_SEQ_PORT_TYPE_APPLICATION);
    if (result < 0)
    {
        QMessageBox::warning(parent_widget, tr("ALSA Error"), tr("failed to open port"));
    }

    snd_seq_nonblock(seq_handle, 1);

    return seq_handle;
}

LiveMidiWorker::LiveMidiWorker(size_t num_channels, QWidget *parent_widget)
        : QObject(nullptr),
          parent_widget(parent_widget),
          num_channels(num_channels)
{}

void LiveMidiWorker::listen_for_midi()
{
    start_midi();
    emit my_finished();
}

void LiveMidiWorker::start_midi()
{
    auto seq_handle = midi_open();
    if (seq_handle == nullptr)
    {
        return;
    }

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
            QThread::msleep(10);
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
