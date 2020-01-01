#include <Windows.h>

#include <midi/midi_input.h>
#include <common.h>

std::map<unsigned int, State> parse_midifile(smf::MidiFile midifile)
{
	// TODO: make track number an argument
	auto const track = midifile[0];
	std::map<unsigned int, State> states;
	// Initial OFF message to turn every thing off
	states.emplace(0, State{});
	State current_state;
	auto const size = track.size();
	for (int event_idx = 0; event_idx < size; ++event_idx)
	{
		auto const event = track[event_idx];
		auto const tick = event.tick;

		if (!(event.isNoteOn() || event.isNoteOff()))
		{
			continue;
		}

		{
			auto const data = event.data();
			auto const bit_idx = (static_cast<int>(data[2]) - midi_note_offset);
			// TODO: something's wrong with my test midi file - this shouldn't be here
			if (bit_idx < 0)
			{
				continue;
			}
			current_state.set(bit_idx, event.isNoteOn());
		}

		// look ahead and merge in any of the next events which are supposed to occur simulatenously
		for (int lookahead_idx = event_idx + 1; lookahead_idx < track.size(); ++lookahead_idx)
		{
			auto const future_event = track[lookahead_idx];
			auto const future_tick = future_event.tick;
			if (future_tick == tick)
			{
				auto const future_data = future_event.data();
				auto const future_bit_idx = (static_cast<int>(future_data[2]) - midi_note_offset);
				if (future_bit_idx < 0)
				{
					continue;
				}
				current_state.set(future_bit_idx, future_event.isNoteOn());
				event_idx += 1;
			}
			else
			{
				break;
			}
		}
		states[tick] = current_state;
	}

	// FInal OFF message to turn every thing off
	states.emplace(track[size - 1].tick + 1, State{});

	return states;
}

void play_midi_data(smf::MidiFile midifile, std::map<unsigned int, State> states, std::optional<serial::Serial> &xbee_serial)
{
	// Transmit messages
	auto const t0 = std::chrono::high_resolution_clock::now();
	for (auto& pair : states)
	{
		auto const& tick = pair.first;
		auto const& onset_ms = static_cast<int>(midifile.getTimeInSeconds(tick) * 1000);
		auto const& state = pair.second;

		auto const t_last = std::chrono::high_resolution_clock::now();
		while (true)
		{
			auto const now = std::chrono::high_resolution_clock::now();
			auto const dt = now - t0;
			auto const dt_to_last = now - t_last;
			if (dt >= std::chrono::milliseconds(onset_ms))
			{
				// transmit
				if (xbee_serial) {
					xbee_serial->write(state.data.data(), message_size);
				}
				break;
			}
		}

	}
}

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
		unsigned int pitch = static_cast<unsigned int>((dwParam1 >> 8) & 0xFF) - midi_note_offset;
		unsigned int channel_number = pitch % num_channels;
		unsigned int suit_number = static_cast<unsigned int>(pitch / num_channels) + 1;
		//char buff[100];
		//snprintf(buff, 100, "%d\n", pitch);
		//OutputDebugString(buff);

		// send a message to the visualizer?
		emit midi_event(suit_number, command, channel_number);
	}
}
