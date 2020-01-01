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
		unsigned int command = dwParam1 & 0xFF;
		unsigned int pitch = static_cast<unsigned int>((dwParam1 >> 8) & 0xFF) - 48;
		unsigned int channel_number = pitch % num_channels;
		unsigned int suit_number = static_cast<unsigned int>(pitch / num_channels) + 1;
		//char buff[100];
		//snprintf(buff, 100, "%d\n", pitch);
		//OutputDebugString(buff);

		// send a message to the visualizer?
		emit midi_event(suit_number, command, channel_number);
	}
}
