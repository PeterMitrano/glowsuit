#include <Windows.h>

#include <midi/midi_input.h>

MidiInputWorker::MidiInputWorker(size_t num_channels, QObject* parent)
	: QObject(parent),
	num_channels(num_channels)
{}

void MidiInputWorker::listen_for_midi() {
	auto return_code = start_midi();
	if (return_code == -1) {
		 //TODO handle failures here
	}
	emit my_finished();
}

int MidiInputWorker::start_midi()
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
		reinterpret_cast<DWORD_PTR>(&MidiInputWorker::StaticCallback),
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

void CALLBACK MidiInputWorker::StaticCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	auto* instance = reinterpret_cast<MidiInputWorker*>(dwInstance);
	return instance->callback(hMidiIn, wMsg, dwParam1, dwParam2);
}

void CALLBACK MidiInputWorker::callback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwParam1, DWORD dwParam2)
{
	if (wMsg == MIM_DATA)
	{
		uint8_t command = dwParam1 & 0xFF;
		uint8_t pitch = ((dwParam1 >> 8) & 0xFF) - 60;
		uint8_t channel_number = pitch % num_channels;
		uint8_t suit_number = pitch + 1u;

		// send a message to the visualizer?
		emit midi_event(suit_number, command, channel_number);
	}
}
