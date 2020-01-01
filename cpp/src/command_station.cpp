// Windows is a special header , it must go first
#include <Windows.h>

#include <bitset>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>

#include <QApplication>
#include <qmainwindow.h>
#include <QPushButton>
#include <QtPlugin>

// TODO: fix this path?
#include <../src/ui_mainwindow.h>

#include <byteset.h>
#include <json.h>
#include <midi/MidiFile.h>
#include <serial/serial.h>
#include <visualizer.h>
#include <main_ui.h>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

using json = nlohmann::json;

using namespace std::chrono_literals;

constexpr int message_size = 6;
constexpr int midi_note_offset = 60;
using State = Byteset<message_size>;

void PrintMidiDevices();


std::map<unsigned int, State> parse_midifile(smf::MidiFile);
void play_midi_data(smf::MidiFile, std::map<unsigned int, State>, std::optional<serial::Serial> const&);
void play_from_live_midi();
std::optional<json> load_suit_description();


int main(int argc, char** argv)
{
	auto const suit_description = load_suit_description();
	auto const num_channels = [&]() {
		if (suit_description) {
			return suit_description.value()["channels"].size();
		}
		// Default size is 6
		return 6u;
	}();

	QApplication app(argc, argv);
	QMainWindow main_window;
	Ui_MainWindow ui;
	ui.setupUi(&main_window);
	Visualizer viz(suit_description, num_channels);
	ui.verticalLayout->addWidget(&viz);
	// TODO: use unique_ptr for Viz here
	MainUI main(ui, &viz, num_channels);
	main.setup_ui();
	main_window.show();
	return app.exec();

	auto const midi_filename = std::string();
	auto const music_filename = std::string();
	auto const serial_port = std::string();

	// Create serial port for writing to the XBee
	std::optional<serial::Serial> xbee_serial;
	if (true) {
		xbee_serial.emplace(serial_port, 57600, serial::Timeout::simpleTimeout(1000));
	}

	if (false) {
		// Parse MIDI file into sequence of serial messages
		smf::MidiFile midifile;
		auto const status = midifile.read(midi_filename);
		if (!status)
		{
			std::cout << "ERROR: couldn't open file\n";
			return EXIT_FAILURE;
		}
		auto const states = parse_midifile(midifile);
		play_midi_data(midifile, states, xbee_serial);
	}
	else {
		play_from_live_midi();
	}

	return EXIT_SUCCESS;
}

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
			auto const id_number = (static_cast<int>(data[1]) - midi_note_offset);
			current_state.set(id_number, event.isNoteOn());
			// TODO: something's wrong with my test midi file - this shouldn't be here
			if (id_number < 0)
			{
				continue;
			}
		}

		// look ahead and merge in any of the next events which are supposed to occur simulatenously
		for (int lookahead_idx = event_idx + 1; lookahead_idx < track.size(); ++lookahead_idx)
		{
			auto const future_event = track[lookahead_idx];
			auto const future_tick = future_event.tick;
			if (future_tick == tick)
			{
				auto const future_data = future_event.data();
				auto const future_id_number = (static_cast<int>(future_data[1]) - midi_note_offset);
				if (future_id_number < 0)
				{
					continue;
				}
				current_state.set(future_id_number, future_event.isNoteOn());
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

void play_midi_data(std::map<unsigned int, State> states, smf::MidiFile midifile, std::optional<serial::Serial> xbee_serial)
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

void play_from_live_midi()
{

}

std::optional<json> load_suit_description()
{
	std::ifstream suit_description_file("suit.json");
	if (!suit_description_file.fail()) {
		// use QT dialog to show error?
		json suit_description;
		suit_description_file >> suit_description;
		return std::optional<json>(suit_description);
	}
	return std::optional<json>{};
}

