#pragma once

#include <string>

#include <QObject>
#include <QString>
#include <QThread>

#include <../src/ui_mainwindow.h>

#include <visualizer.h>

QString select_music_file(QWidget* parent);

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

	void music_file_button_clicked();

public:

	Ui_MainWindow ui;
	Visualizer* viz;
	QString music_filename;
	QThread midi_thread;
	size_t num_channels;
};
