#include <Windows.h>

#include <QFileDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>

#include <audio/play_music.h>
#include <common.h>
#include <main_ui.h>


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
	QObject::connect(ui.select_midi_file_button, &QPushButton::clicked, this, &MainUI::midi_file_button_clicked);
	QObject::connect(ui.live_checkbox, &QCheckBox::stateChanged, this, &MainUI::live_midi_changed);

	// start a thread for receiving MIDI
	LiveMidiWorker* worker = new LiveMidiWorker(num_channels);
	worker->moveToThread(&live_midi_thread);
	QObject::connect(&live_midi_thread, &QThread::started, worker, &LiveMidiWorker::listen_for_midi);
	QObject::connect(worker, &LiveMidiWorker::my_finished, &live_midi_thread, &QThread::quit);
	QObject::connect(worker, &LiveMidiWorker::midi_event, viz, &Visualizer::on_live_midi_event);
	live_midi_thread.start();


	// Create serial port for writing to the XBee
	ports = serial::list_ports();
	for (auto& port : ports) {
		if (port.description.find("Serial Port") != std::string::npos) {
			ui.xbee_port_combobox->insertItem(0, QString::fromStdString(port.port));
		}
	}

	QObject::connect(ui.xbee_port_combobox, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainUI::xbee_port_changed);
}

MainUI::~MainUI()
{
	live_midi_thread.requestInterruption();
	live_midi_thread.quit();
	live_midi_thread.wait();
}

void MainUI::play_pause_clicked(bool checked)
{
	// checked means play
	if (checked)
	{
		// TODO: make these requests to play/pause occur on a seperate thread
		//play_music(music_filename.toStdString());

		// start a thread for receiving MIDI
		MidiFileWorker* worker = new MidiFileWorker(num_channels, midifile, states, xbee_serial);
		worker->moveToThread(&midi_file_thread);
		QObject::connect(&midi_file_thread, &QThread::started, worker, &MidiFileWorker::play_midi_data);
		QObject::connect(worker, &MidiFileWorker::my_finished, &midi_file_thread, &QThread::quit);
		QObject::connect(worker, &MidiFileWorker::midi_event, viz, &Visualizer::on_midi_file_event);
		midi_file_thread.start();
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
		ui.music_filename_label->setText(music_filename);
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

void MainUI::midi_file_button_clicked()
{
	midi_filename = select_midi_file(ui.main);
	if (!midi_filename.isNull())
	{
		ui.midi_filename_label->setText(midi_filename);

		// Parse MIDI file into sequence of serial messages
		auto const result = midifile.read(midi_filename.toStdString());
		states = parse_midifile(midifile);

		if (!ui.live_checkbox->isChecked())
		{
			ui.play_pause_button->setEnabled(true);
		}
	}
}

QString select_midi_file(QWidget* parent)
{
	return QFileDialog::getOpenFileName(parent, "Open MIDI File", QString(), "*.mid");
}


void MainUI::live_midi_changed(int state)
{
	if (state == Qt::Checked)
	{
		ui.play_pause_button->setEnabled(false);
		ui.midi_indicator_label->setEnabled(true);
		viz->viz_from_live_midi = true;
	}
	else if (state == Qt::Unchecked)
	{
		viz->viz_from_live_midi = false;
		ui.midi_indicator_label->setEnabled(false);
		if (!music_filename.isNull()) {
			// play pause should only be enabled if BOTH live midi is unchecked AND there's a valid music file
			ui.play_pause_button->setEnabled(true);
		}
	}
}


void MainUI::xbee_port_changed(int index)
{

	auto port = ports[index];
	xbee_serial.emplace(port.port, baud_rate, serial::Timeout::simpleTimeout(1000));
}
