#pragma once

#include <Windows.h>

#include <conio.h>
#include <mmsystem.h>
#include <SDKDDKVer.h>

#include <QThread>

class MidiInputWorker : public QObject
{
	Q_OBJECT

public:
	MidiInputWorker(size_t num_channels, QObject* parent = nullptr);

	void CALLBACK callback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwParam1, DWORD dwParam2);

	int start_midi();

	static void CALLBACK StaticCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

public slots:
	void listen_for_midi();

signals:
	void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

	void my_finished();

private:
	size_t num_channels;

};

