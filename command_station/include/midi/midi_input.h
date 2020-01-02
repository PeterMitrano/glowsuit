#pragma once

#include <Windows.h>

#include <conio.h>
#include <map>
#include <mmsystem.h>
#include <optional>
#include <SDKDDKVer.h>

#include <QThread>

#include <byteset.h>
#include <serial/serial.h>
#include <midi/MidiFile.h>

constexpr int message_size = 6;
using State = Byteset<message_size>;

std::map<unsigned int, State> parse_midifile(smf::MidiFile, int octave_offset);
void play_midi_data(smf::MidiFile midifile, std::map<unsigned int, State> states, std::optional<serial::Serial>& xbee_serial);

class LiveMidiWorker : public QObject
{
	Q_OBJECT

public:
	LiveMidiWorker(size_t num_channels, QObject* parent = nullptr);

	void CALLBACK callback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwParam1, DWORD dwParam2);

	int start_midi();

	static void CALLBACK StaticCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

	serial::Serial* xbee_serial{ nullptr };

public slots:
	void listen_for_midi();

	void octave_spinbox_changed(int value);

signals:
	void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

	void my_finished();

private:
	size_t num_channels;
	int octave_offset;

	State current_state;

};


class MidiFileWorker : public QObject
{
	Q_OBJECT

public:
	MidiFileWorker(size_t num_channels, QObject* parent = nullptr);

	serial::Serial* xbee_serial{ nullptr };

public slots:
	void play_midi_data(QString midi_filename, int octave_offset);

	void octave_spinbox_changed(int value);

signals:
	void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

	void my_finished();

private:
	size_t num_channels;
	int octave_offset;
};

