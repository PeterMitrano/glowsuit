#pragma once

#include <Windows.h>

#include <conio.h>
#include <map>
#include <mmsystem.h>
#include <optional>
#include <SDKDDKVer.h>

#include <QThread>

#include <serial/serial.h>
#include <midi/MidiFile.h>
#include <midi/midi_common.h>

class LiveMidiWorker : public QObject
{
	Q_OBJECT

public:
	LiveMidiWorker(size_t num_channels, QWidget* parent_widget);

	void CALLBACK callback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwParam1, DWORD dwParam2);

	int start_midi();

	static void CALLBACK StaticCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

	serial::Serial* xbee_serial{ nullptr };

public slots:
	void listen_for_midi();

	void octave_spinbox_changed(int value);

signals:
	void any_event();

	void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

	void my_finished();

private:
	QWidget* parent_widget;
	size_t num_channels;
	int octave_offset{ 0 };

	State current_state;

};
