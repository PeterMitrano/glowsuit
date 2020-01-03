#include <midi/midi_input.h>
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
	HMIDIIN hMidiDevice = nullptr;
	DWORD nMidiPort = 0;
	UINT nMidiDeviceNum;
	MMRESULT rv;

	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		//fprintf(stderr, "midiInGetNumDevs() return 0...");
		return -1;
	}

	rv = midiInOpen(&hMidiDevice,
		nMidiPort,
		reinterpret_cast<DWORD_PTR>(&LiveMidiWorker::StaticCallback),
		reinterpret_cast<DWORD_PTR>(this),
		CALLBACK_FUNCTION);
	if (rv != MMSYSERR_NOERROR) {
		//fprintf(stderr, "midiInOpen() failed...rv=%d", rv);
		return -1;
	}

	midiInStart(hMidiDevice);

	while (true) {
		if (QThread::currentThread()->isInterruptionRequested())
		{
			break;
		}
		if (!_kbhit()) {
			Sleep(100);
			continue;
		}
	}

	midiInStop(hMidiDevice);
	midiInClose(hMidiDevice);

	return 0;
}

void CALLBACK LiveMidiWorker::StaticCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	auto* instance = reinterpret_cast<LiveMidiWorker*>(dwInstance);
	return instance->callback(hMidiIn, wMsg, dwParam1, dwParam2);
}

void CALLBACK LiveMidiWorker::callback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwParam1, DWORD dwParam2)
{
	if (wMsg == MIM_DATA)
	{
		unsigned int command = dwParam1 & 0xFF;
		int bit_idx = static_cast<int>((dwParam1 >> 8) & 0xFF) - midi_note_offset + 12 * octave_offset;

		if (bit_idx < 0) {
			return;
		}

		unsigned int channel_number = bit_idx % num_channels;
		unsigned int suit_idx = static_cast<unsigned int>(bit_idx / num_channels);
		unsigned int suit_number = suit_idx + 1;

		// send a message to the visualizer
		emit midi_event(suit_number, command, channel_number);

		// update the current state and send the data
		if (command == 128) {
			current_state.set(bit_idx, false);
		}
		else if (command == 144) {
			current_state.set(bit_idx, true);
		}
		else {
			// just skip if it's not NOTE ON or NOTE OFF
			return;
		}

		// transmit to suits
		if (xbee_serial != nullptr) {
			xbee_serial->write(current_state.data.data(), message_size);
		}
	}
}

void LiveMidiWorker::octave_spinbox_changed(int value)
{
	octave_offset = value;
}
