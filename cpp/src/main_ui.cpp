#include <Windows.h>

#include <QFileDialog>
#include <QPushButton>
#include <QCheckBox>

#include <main_ui.h>
#include <audio/play_music.h>
#include <midi/midi_input.h>


MainUI::MainUI(Ui_MainWindow const ui, Visualizer* const viz, size_t num_channels)
	: ui(ui),
	viz(viz),
	num_channels(num_channels)
{
}

void MainUI::setup_ui()
{
	QObject::connect(ui.front_button, &QPushButton::clicked, viz, &Visualizer::front_status_clicked);
	QObject::connect(ui.back_button, &QPushButton::clicked, viz, &Visualizer::back_status_clicked);
	QObject::connect(ui.play_pause_button, &QPushButton::clicked, this, &MainUI::play_pause_clicked);
	QObject::connect(ui.select_music_file_button, &QPushButton::clicked, this, &MainUI::music_file_button_clicked);
	QObject::connect(ui.live_checkbox, &QCheckBox::stateChanged, this, &MainUI::live_midi_changed);

	// start a thread for receiving MIDI
	MidiInputWorker* worker = new MidiInputWorker(num_channels);
	worker->moveToThread(&midi_thread);
	QObject::connect(&midi_thread, &QThread::started, worker, &MidiInputWorker::listen_for_midi);
	QObject::connect(worker, &MidiInputWorker::my_finished, &midi_thread, &QThread::quit);
	midi_thread.start();

	// when a midi event is received, MidiInputWorker will emit an event
	QObject::connect(worker, &MidiInputWorker::midi_event, viz, &Visualizer::on_midi_event);
}

MainUI::~MainUI()
{
	midi_thread.requestInterruption();
	midi_thread.quit();
	midi_thread.wait();
}

void MainUI::play_pause_clicked(bool checked)
{
	// checked means play
	if (checked)
	{
		// TODO: make these requres to play/pause occur on a seperate thread
		//play_music(music_filename.toStdString());
	}
	else {
		//pause_music();
	}
}

void MainUI::music_file_button_clicked()
{
	music_filename = select_music_file(ui.main);
	if (!music_filename.isNull())
	{
		if (!ui.live_checkbox->isChecked())
		{
			ui.play_pause_button->setEnabled(true);
		}
	}
}

QString select_music_file(QWidget* parent)
{
	return QFileDialog::getOpenFileName(parent, "Open Music File", QString(), "*.wav");
}


void MainUI::live_midi_changed(int state)
{
	if (state == Qt::Checked)
	{
		ui.play_pause_button->setEnabled(false);
		ui.midi_indicator_label->setEnabled(true);
	}
	else if (state == Qt::Unchecked)
	{
		ui.midi_indicator_label->setEnabled(false);
		if (!music_filename.isNull()) {
			// play pause should only be enabled if BOTH live midi is unchecked AND there's a valid music file
			ui.play_pause_button->setEnabled(true);
		}
	}
}

