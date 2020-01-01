#pragma once

#include <string>
#include <map>
#include <optional>

#include <QObject>
#include <QString>
#include <QThread>

#include <../src/ui_mainwindow.h>

#include <midi/midi_input.h>
#include <serial/serial.h>
#include <visualizer.h>

QString select_music_file(QWidget* parent);

QString select_midi_file(QWidget* parent);

class MainUI : public QObject
{
	Q_OBJECT

public:
	virtual ~MainUI();

	/// viz must not be null
	MainUI(Ui_MainWindow ui, Visualizer* viz, size_t num_channels);

	void setup_ui();

public slots:
	void live_midi_changed(int state);

	void play_pause_clicked(bool checked);

	void midi_file_button_clicked();

	void music_file_button_clicked();

	void xbee_port_changed(int index);

public:

	Ui_MainWindow ui;
	Visualizer* viz;
	QString music_filename;
	QString midi_filename;
	QThread live_midi_thread;
	QThread music_thread;
	QThread midi_file_thread;
	size_t num_channels;
	smf::MidiFile midifile;
	std::map<unsigned int, State> states;
	std::optional<serial::Serial> xbee_serial;
	std::vector<serial::PortInfo> ports;
};
