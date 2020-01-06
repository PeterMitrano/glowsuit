#include <QAbstractEventDispatcher>
#include <QMessageBox>
#include <QThread>

#include <midi/osx/midi_input_osx.h>
#include <sstream>
#include <common.h>

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
    while (true)
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            break;
        }

        // since this is an infinite loop, in order to receive Qt signals we need to call this manually
        thread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

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
