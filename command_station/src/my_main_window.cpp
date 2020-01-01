#include <Windows.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QPushButton>
#include <QSpinBox>

#include <common.h>
#include <my_main_window.h>


MyMainWindow::MyMainWindow(Ui_MainWindow const ui, Visualizer* const viz, size_t num_channels)
	: ui(ui),
	viz(viz),
	num_channels(num_channels),
	live_midi_worker(num_channels),
	midi_file_worker(num_channels),
	music_worker()
{
}

void MyMainWindow::setup_ui()
{
	QObject::connect(ui.front_button, &QPushButton::clicked, viz, &Visualizer::front_status_clicked);
	QObject::connect(ui.back_button, &QPushButton::clicked, viz, &Visualizer::back_status_clicked);
	QObject::connect(ui.play_pause_button, &QPushButton::clicked, this, &MyMainWindow::play_pause_clicked);
	QObject::connect(ui.select_music_file_button, &QPushButton::clicked, this, &MyMainWindow::music_file_button_clicked);
	QObject::connect(ui.select_midi_file_button, &QPushButton::clicked, this, &MyMainWindow::midi_file_button_clicked);
	QObject::connect(ui.live_checkbox, &QCheckBox::stateChanged, this, &MyMainWindow::live_midi_changed);
	// FIXME: this is broken because the workers are nullptr at the moment
	QObject::connect(ui.octave_spinbox, qOverload<int>(&QSpinBox::valueChanged), &live_midi_worker, &LiveMidiWorker::octave_spinbox_changed);
	QObject::connect(ui.octave_spinbox, qOverload<int>(&QSpinBox::valueChanged), &midi_file_worker, &MidiFileWorker::octave_spinbox_changed);

	// Thread for music
	music_worker.moveToThread(&music_thread);
	QObject::connect(this, &MyMainWindow::play_music, &music_worker, &MusicWorker::play_music);
	QObject::connect(&music_worker, &MusicWorker::my_finished, &music_thread, &QThread::quit);
	music_thread.start();

	// start a thread for receiving MIDI
	midi_file_worker.moveToThread(&midi_file_thread);
	QObject::connect(this, &MyMainWindow::play_midi_data, &midi_file_worker, &MidiFileWorker::play_midi_data);
	QObject::connect(&midi_file_worker, &MidiFileWorker::my_finished, &midi_file_thread, &QThread::quit);
	QObject::connect(&midi_file_worker, &MidiFileWorker::midi_event, viz, &Visualizer::on_midi_file_event);
	midi_file_thread.start();

	// start a thread for receiving MIDI
	live_midi_worker.moveToThread(&live_midi_thread);
	QObject::connect(&live_midi_thread, &QThread::started, &live_midi_worker, &LiveMidiWorker::listen_for_midi);
	QObject::connect(&live_midi_worker, &LiveMidiWorker::my_finished, &live_midi_thread, &QThread::quit);
	QObject::connect(&live_midi_worker, &LiveMidiWorker::midi_event, viz, &Visualizer::on_live_midi_event);
	live_midi_thread.start();

	// Create serial port for writing to the XBee
	ports = serial::list_ports();
	for (auto& port : ports) {
		if (port.description.find("Serial Port") != std::string::npos) {
			ui.xbee_port_combobox->insertItem(0, QString::fromStdString(port.port));
		}
	}

	QObject::connect(ui.xbee_port_combobox, qOverload<int>(&QComboBox::currentIndexChanged), this, &MyMainWindow::xbee_port_changed);

	restore_settings();
}

MyMainWindow::~MyMainWindow()
{
	live_midi_thread.requestInterruption();
	live_midi_thread.quit();
	live_midi_thread.wait();

	midi_file_thread.quit();
	midi_file_thread.wait();

	music_thread.quit();
	music_thread.wait();
}

void MyMainWindow::play_pause_clicked(bool checked)
{
	// checked means play
	if (checked)
	{
		emit play_music(music_filename);
		emit play_midi_data(midifile, states);
	}
	else {
		//pause_music();
	}
}

void MyMainWindow::music_file_button_clicked()
{
	music_filename = select_music_file(ui.main);
	if (!music_filename.isNull())
	{
		ui.music_filename_label->setText(music_filename);
		if (!ui.live_checkbox->isChecked() && !midi_filename.isNull())
		{
			ui.play_pause_button->setEnabled(true);
		}
	}
}

QString select_music_file(QWidget* parent)
{
	return QFileDialog::getOpenFileName(parent, "Open Music File", QString(), "*.wav");
}

void MyMainWindow::midi_file_button_clicked()
{
	midi_filename = select_midi_file(ui.main);
	if (!midi_filename.isNull())
	{
		ui.midi_filename_label->setText(midi_filename);

		// Parse MIDI file into sequence of serial messages
		auto const result = midifile.read(midi_filename.toStdString());
		states = parse_midifile(midifile, ui.octave_spinbox->value());

		if (!ui.live_checkbox->isChecked() && !music_filename.isNull())
		{
			ui.play_pause_button->setEnabled(true);
		}
	}
}

QString select_midi_file(QWidget* parent)
{
	return QFileDialog::getOpenFileName(parent, "Open MIDI File", QString(), "*.mid");
}

void MyMainWindow::live_midi_changed(int state)
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

void MyMainWindow::xbee_port_changed(int index)
{
	auto port = ports[index];
	xbee_serial = new serial::Serial(port.port, baud_rate, serial::Timeout::simpleTimeout(1000));

	// TODO: is this an error? possible data race
	live_midi_worker.xbee_serial = xbee_serial;
	midi_file_worker.xbee_serial = xbee_serial;
}

void MyMainWindow::closeEvent(QCloseEvent* event)
{
	save_settings();
	event->accept();
}

void MyMainWindow::save_settings() {
	settings->setValue("gui/octave/value", ui.octave_spinbox->value());
	//settings->setValue("gui/info_tabs", ui_->info_tabs->currentIndex());
	//settings->setValue("gui/static_", ui_->static_checkbox->isChecked());
	//settings->setValue("gui/real_time_value", ui_->real_time_factor_spinner->value());
	//settings->setValue("gui/kp", ui_->kp_spinbox->value());
	//settings->setValue("gui/ki", ui_->ki_spinbox->value());
	//settings->setValue("gui/kd", ui_->kd_spinbox->value());
	//settings->setValue("gui/kff_offset", ui_->kff_offset_spinbox->value());
	//settings->setValue("gui/kff_scale", ui_->kff_scale_spinbox->value());
}

void MyMainWindow::restore_settings() {
	QCoreApplication::setOrganizationName("Photonix");
	QCoreApplication::setApplicationName("Glowsuit");
	settings = new QSettings();

	ui.octave_spinbox->setValue(settings->value("gui/octave/value").toInt());
	//const QByteArray splitter_state = settings->value("gui/main_splitter").toByteArray();
	//if (!splitter_state.isEmpty()) {
	//	ui_->main_splitter->restoreState(splitter_state);
	//}

	//const int info_tab_index = settings->value("gui/info_tabs").toInt();
	//ui_->info_tabs->setCurrentIndex(info_tab_index);

	//maze_files_dir_ = settings->value("gui/maze_files_directory").toString();
	//default_maze_file_name_ = settings->value("gui/default_maze_file_name").toString();
	//LoadDefaultMaze();

	//ui_->static_checkbox->setChecked(settings->value("gui/static_").toBool());

	//ui_->real_time_factor_spinner->setValue(settings->value("gui/real_time_value").toDouble());
	//ui_->kp_spinbox->setValue(settings->value("gui/kp").toDouble());
	//ui_->ki_spinbox->setValue(settings->value("gui/ki").toDouble());
	//ui_->kd_spinbox->setValue(settings->value("gui/kd").toDouble());
	//ui_->kff_offset_spinbox->setValue(settings->value("gui/kff_offset").toDouble());
	//ui_->kff_scale_spinbox->setValue(settings->value("gui/kff_scale").toDouble());
}

