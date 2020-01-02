#pragma once

#include <string>
#include <map>
#include <optional>

#include <QCloseEvent>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QWidget>

#include <../src/ui_mainwidget.h>

#include <audio/play_music.h>
#include <midi/midi_input.h>
#include <serial/serial.h>
#include <visualizer.h>

std::optional<json> load_suit_description();

QString select_music_file(QWidget* parent);

QString select_midi_file(QWidget* parent);

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	virtual ~MainWidget();

	MainWidget(std::optional<json> suit_description, unsigned int num_channels, QWidget* parent = nullptr);

	void setup_ui();

	void save_settings();

	void restore_settings();

	void closeEvent(QCloseEvent* event) override;

public slots:
	void live_midi_changed(int state);

	void play_pause_clicked(bool checked);

	void midi_file_button_clicked();

	void music_file_button_clicked();

	void xbee_port_changed(int index);

signals:
	void play_music(QString music_filename);

	void play_midi_data(smf::MidiFile midifile, std::map<unsigned int, State> states);

public:
	Visualizer viz;
	LiveMidiWorker live_midi_worker;
	MidiFileWorker midi_file_worker;
	MusicWorker music_worker;
	QString music_filename;
	QString midi_filename;
	QThread live_midi_thread;
	QThread music_thread;
	QThread midi_file_thread;
	size_t num_channels;
	smf::MidiFile midifile;
	std::map<unsigned int, State> states;
	// TODO: better than raw pointer? problem with optional is serial::Serial is not copyable
	serial::Serial* xbee_serial{ nullptr };
	std::vector<serial::PortInfo> ports;
	QSettings* settings;


private:
	Ui_MainWidget ui;
};
