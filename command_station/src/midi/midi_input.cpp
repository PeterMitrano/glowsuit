#include <QAbstractEventDispatcher>
#include <QMessageBox>
#include <QString>
#include <QThread>

#include <midi/midi_input.h>
#include <sstream>
#include <common.h>

void error_callback(RtMidiError::Type type, const std::string &errorText, void *userData)
{
    auto const context = (LiveMidiWorker *) (userData);
    context->show_midi_warning(QString::fromStdString(errorText));
}

LiveMidiWorker::LiveMidiWorker(size_t num_channels, QWidget *parent_widget)
        : QObject(nullptr),
          parent_widget(parent_widget),
          num_channels(num_channels)
{
    midiin.setErrorCallback(&error_callback, this);
}

void LiveMidiWorker::show_midi_warning(QString message)
{
    std::cout << message.toStdString() << '\n';
    //QMessageBox::warning(parent_widget,
    //    QString("MIDI Error"),
    //    message
    //);
}

void LiveMidiWorker::listen_for_midi()
{
    start_midi();
    emit my_finished();
}

void LiveMidiWorker::start_midi()
{
    // Check available ports.
    midiin.openVirtualPort("glowsuit:in");

    if (!midiin.isPortOpen())
    {
        return;
    }

    // Ignore sysex, timing, or active sensing messages.
    midiin.ignoreTypes(true, true, true);

    while (true)
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            break;
        }

        // since this is an infinite loop, in order to receive Qt signals we need to call this manually
        thread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

        // 0 means no message we ready
        std::vector<unsigned char> message;
        if (midiin.getMessage(&message) == 0.0)
        {
            continue;
        }

        emit any_event();

        auto const command = static_cast<int>(message[0]);

        if (command != midi_note_off && command != midi_note_on)
        {
            continue;
        }

        auto const pitch = static_cast<int>(message[1]);
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
        if (command == midi_note_off)
        {
            current_state.set(bit_idx, false);
        } else if (command == midi_note_on)
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
            auto const[packet, size] = make_packet(current_state);
            xbee_serial->write(packet.data(), size);
        }
    }
}

void LiveMidiWorker::octave_spinbox_changed(int value)
{
    octave_offset = value;
}
