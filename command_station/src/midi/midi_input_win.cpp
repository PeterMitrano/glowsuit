#include <Windows.h>

#include <midi/midi_input.h>
#include <sstream>
#include <common.h>

std::map<unsigned int, State> parse_midifile(smf::MidiFile midifile, int octave_offset)
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
			// why are all of these off by an octave?
			auto const bit_idx = (static_cast<int>(data[1]) - midi_note_offset + 12 * octave_offset);
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
				auto const future_bit_idx = (static_cast<int>(future_data[1]) - midi_note_offset + 12 * octave_offset);
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

MidiFileWorker::MidiFileWorker(size_t num_channels,
	QObject* parent)
	: QObject(parent),
	num_channels(num_channels)
{}

// TODO: need to register the smf::MifiFile for QT transport
void MidiFileWorker::play_midi_data(smf::MidiFile midifile, std::map<unsigned int, State> states)
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
				// visualizer 
				for (auto suit_idx{ 0u }; suit_idx < num_suits; ++suit_idx) {
					auto const byte = state.data[suit_idx];
					for (auto bit_idx{ 0u }; bit_idx < 8; ++bit_idx)
					{
						auto const bit = (byte >> bit_idx) & 0x1;
						auto const command = bit == 1 ? 144 : 128;
						auto const channel_number = bit_idx;

						emit midi_event(suit_idx + 1, command, channel_number);
					}
				}

				// transmit 
				if (xbee_serial != nullptr) {
					xbee_serial->write(state.data.data(), message_size);
				}
				break;
			}
		}
	}
}


void MidiFileWorker::octave_spinbox_changed(int value)
{
	octave_offset = value;
}
