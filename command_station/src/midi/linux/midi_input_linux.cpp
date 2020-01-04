#include <QThread>

#include <midi/linux/midi_input_linux.h>
#include <sstream>
#include <common.h>


LiveMidiWorker::LiveMidiWorker(size_t num_channels, QObject* parent)
	: QObject(parent),
	num_channels(num_channels)
{}

void LiveMidiWorker::listen_for_midi() {
	auto return_code = start_midi();
	if (return_code == -1) {
		//TODO handle failures here
	}
	emit my_finished();
}

int LiveMidiWorker::start_midi()
{
	while (true) {
		if (QThread::currentThread()->isInterruptionRequested())
		{
			break;
		}
		// read midi
	}

	return 0;
}

void LiveMidiWorker::octave_spinbox_changed(int value)
{
	octave_offset = value;
}
