#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringBuilder>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QTime>

#include <common.h>
#include <main_widget.h>
#include <midi/midi_file_player.h>

MainWidget::MainWidget(QWidget *parent)
        : QWidget(parent)
{
    ui.setupUi(this);

    ui.play_button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui.stop_button->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    connect(ui.play_button, &QAbstractButton::clicked, this, &MainWidget::play_pause_clicked);
    connect(ui.stop_button, &QAbstractButton::clicked, this, &MainWidget::stop);

    // these are pointers because you can't use "this" in an the constructor initializer list
    visualizer = new Visualizer(this);
    auto const num_channels = visualizer->load_suit();
    ui.visualizer_group->layout()->addWidget(visualizer);
    live_midi_worker = new LiveMidiWorker(num_channels, this);
    music_player = new QMediaPlayer(this);

    midi_file_player = new MidiFilePlayer();
    midi_file_player->start_thread();
    connect(this, &MainWidget::midi_file_changed, midi_file_player, &MidiFilePlayer::midi_file_changed);
    connect(midi_file_player, &MidiFilePlayer::midi_event, visualizer, &Visualizer::on_midi_file_event);
    ui.player_slider->setRange(0, static_cast<int>(music_player->duration() / 1000));

    connect(ui.player_slider, &QSlider::sliderMoved, this, &MainWidget::seek);
    connect(ui.player_slider, &QSlider::sliderMoved, midi_file_player, &MidiFilePlayer::seek);
    connect(music_player, &QMediaPlayer::durationChanged, this, &MainWidget::duration_changed);
    connect(music_player, &QMediaPlayer::positionChanged, this, &MainWidget::position_changed);
    connect(music_player, &QMediaPlayer::mediaStatusChanged, this, &MainWidget::status_changed);
    connect(music_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this,
            &MainWidget::display_error_message);

    connect(this, &MainWidget::play, music_player, &QMediaPlayer::play);
    connect(this, &MainWidget::pause, music_player, &QMediaPlayer::pause);
    connect(this, &MainWidget::stop, music_player, &QMediaPlayer::stop);
    connect(this, &MainWidget::play, midi_file_player, &MidiFilePlayer::play);
    connect(this, &MainWidget::pause, midi_file_player, &MidiFilePlayer::pause);
    connect(this, &MainWidget::stop, midi_file_player, &MidiFilePlayer::stop);
    connect(music_player, &QMediaPlayer::stateChanged, this, &MainWidget::set_state);

    connect(ui.front_button, &QPushButton::clicked, visualizer, &Visualizer::front_status_clicked);
    connect(ui.back_button, &QPushButton::clicked, visualizer, &Visualizer::back_status_clicked);
    connect(midi_file_player, &MidiFilePlayer::track_range_changed, this,
            &MainWidget::track_range_changed);
    connect(ui.track_spinbox, qOverload<int>(&QSpinBox::valueChanged), midi_file_player,
            &MidiFilePlayer::track_changed);
    connect(ui.scale_spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), visualizer,
            &Visualizer::viz_scale_changed);
    connect(ui.select_music_file_button, &QPushButton::clicked, this, &MainWidget::music_file_button_clicked);
    connect(ui.select_midi_file_button, &QPushButton::clicked, this, &MainWidget::midi_file_button_clicked);
    connect(ui.live_checkbox, &QCheckBox::stateChanged, this, &MainWidget::live_midi_changed);
    connect(midi_file_player, &MidiFilePlayer::event_count_changed, this, &MainWidget::event_count_changed);
    connect(ui.octave_spinbox, qOverload<int>(&QSpinBox::valueChanged), midi_file_player,
            &MidiFilePlayer::octave_spinbox_changed);

    set_state(music_player->state());

    // start a thread for receiving MIDI
    live_midi_worker->moveToThread(&live_midi_thread);
    connect(&live_midi_thread, &QThread::started, live_midi_worker, &LiveMidiWorker::listen_for_midi);
    connect(live_midi_worker, &LiveMidiWorker::my_finished, &live_midi_thread, &QThread::quit);
    connect(live_midi_worker, &LiveMidiWorker::midi_event, visualizer, &Visualizer::on_live_midi_event);
    connect(live_midi_worker, &LiveMidiWorker::any_event, this, &MainWidget::any_event);
    live_midi_thread.start();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWidget::update_serial_port_list);
    timer->start(1000);

    QObject::connect(ui.xbee_port_combobox, qOverload<int>(&QComboBox::currentIndexChanged), this,
                     &MainWidget::xbee_port_changed);

    restore_settings();

    if (!music_player->isAvailable())
    {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                "Please check the media service plugins are installed."));

        ui.midi_file_group->setEnabled(false);
    }

    // FIXME: this isn't working, and MIDI if off by something on windows?!
    connect(ui.octave_spinbox, qOverload<int>(&QSpinBox::valueChanged), live_midi_worker,
            &LiveMidiWorker::octave_spinbox_changed);
}

MainWidget::~MainWidget()
{
    live_midi_thread.requestInterruption();
    live_midi_thread.quit();
    live_midi_thread.wait();

    midi_player_thread.requestInterruption();
    midi_player_thread.quit();
    midi_player_thread.wait();
}

void MainWidget::music_file_button_clicked()
{
    // any widget will do for a parent here
    music_filename = select_music_file(ui.select_music_file_button);
    if (!music_filename.isNull())
    {
        // load new media and set the label
        ui.music_filename_label->setText(music_filename);
        music_player->setMedia(QUrl::fromLocalFile(music_filename));
    }
}

QString select_music_file(QWidget *parent)
{
    return QFileDialog::getOpenFileName(parent, "Open Music File", QString(), "*.wav");
}

void MainWidget::midi_file_button_clicked()
{
    midi_filename = select_midi_file(ui.select_midi_file_button);
    if (!midi_filename.isNull())
    {
        ui.midi_filename_label->setText(midi_filename);
        emit midi_file_changed(midi_filename);
    }
}

QString select_midi_file(QWidget *parent)
{
    return QFileDialog::getOpenFileName(parent, "Open MIDI File", QString(), "*.mid");
}

void MainWidget::live_midi_changed(int state)
{
    if (state == Qt::Checked)
    {
        visualizer->viz_from_live_midi = true;
        ui.midi_file_group->setEnabled(false);
        emit stop();
    } else if (state == Qt::Unchecked)
    {
        visualizer->viz_from_live_midi = false;
        ui.midi_file_group->setEnabled(true);
    }
}

void MainWidget::xbee_port_changed(int index)
{
    auto port = ports[index];
    xbee_serial = new serial::Serial(port.port, baud_rate, serial::Timeout::simpleTimeout(1000));

    // TODO: is this an error? possible data race
    live_midi_worker->xbee_serial = xbee_serial;
    midi_file_player->xbee_serial = xbee_serial;
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    save_settings();
    event->accept();
}

void MainWidget::save_settings()
{
    settings->setValue("gui/octave", ui.octave_spinbox->value());
    settings->setValue("gui/live_checkbox", ui.live_checkbox->isChecked());
    settings->setValue("gui/scale", ui.scale_spinbox->value());
    settings->setValue("gui/track", ui.track_spinbox->value());
    settings->setValue("files/music", music_filename);
    settings->setValue("files/midi", midi_filename);
}

void MainWidget::restore_settings()
{
    QCoreApplication::setOrganizationName("Photonix");
    QCoreApplication::setApplicationName("Glowsuit");
    settings = new QSettings();

    music_filename = settings->value("files/music").toString();
    ui.music_filename_label->setText(music_filename);
    midi_filename = settings->value("files/midi").toString();
    ui.midi_filename_label->setText(midi_filename);

    music_player->setMedia(QUrl::fromLocalFile(music_filename));
    emit midi_file_changed(midi_filename);

    ui.octave_spinbox->setValue(settings->value("gui/octave").toInt());
    ui.scale_spinbox->setValue(settings->value("gui/scale").toDouble());
    ui.live_checkbox->setChecked(settings->value("gui/live_checkbox").toBool());
    ui.track_spinbox->setValue(settings->value("gui/track").toInt());

    emit visualizer->viz_scale_changed(ui.scale_spinbox->value());

    ui.midi_file_group->setEnabled(!ui.live_checkbox->isChecked());
}

QMediaPlayer::State MainWidget::state() const
{
    return player_state;
}

void MainWidget::set_state(QMediaPlayer::State state)
{
    if (state != player_state)
    {
        player_state = state;

        switch (state)
        {
            case QMediaPlayer::StoppedState:
                ui.stop_button->setEnabled(false);
                ui.play_button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
                break;
            case QMediaPlayer::PlayingState:
                ui.stop_button->setEnabled(true);
                ui.play_button->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
                break;
            case QMediaPlayer::PausedState:
                ui.stop_button->setEnabled(true);
                ui.play_button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
                break;
        }
    }
}

void MainWidget::play_pause_clicked()
{
    switch (player_state)
    {
        case QMediaPlayer::StoppedState:
        case QMediaPlayer::PausedState:
            emit play();
            break;
        case QMediaPlayer::PlayingState:
            emit pause();
            break;
    }
}

void MainWidget::seek(int seconds)
{
    music_player->setPosition(seconds * 1000);
}

void MainWidget::duration_changed(qint64 duration)
{
    this->duration = duration / 1000;
    ui.player_slider->setMaximum(this->duration);
}

void MainWidget::position_changed(qint64 progress)
{
    if (!ui.player_slider->isSliderDown())
        ui.player_slider->setValue(progress / 1000);

    update_duration_info(progress / 1000);
}

void MainWidget::update_duration_info(qint64 currentInfo)
{
    QString time_str;
    if (currentInfo || duration)
    {
        QTime current_time((currentInfo / 3600) % 60, (currentInfo / 60) % 60, currentInfo % 60,
                           (currentInfo * 1000) % 1000);
        QTime total_time((duration / 3600) % 60, (duration / 60) % 60, duration % 60, (duration * 1000) % 1000);
        QString format = "mm:ss";
        if (duration > 3600)
        {
            format = "hh:mm:ss";
        }
        time_str = current_time.toString(format) + " / " + total_time.toString(format);
    }
    ui.player_time_label->setText(time_str);
}

void MainWidget::status_changed(QMediaPlayer::MediaStatus status)
{
    handle_cursor(status);

    // handle status message
    switch (status)
    {
        case QMediaPlayer::UnknownMediaStatus:
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::LoadedMedia:
            break;
        case QMediaPlayer::LoadingMedia:
            break;
        case QMediaPlayer::BufferingMedia:
        case QMediaPlayer::BufferedMedia:
            break;
        case QMediaPlayer::StalledMedia:
            break;
        case QMediaPlayer::EndOfMedia:
            QApplication::alert(this);
            break;
        case QMediaPlayer::InvalidMedia:
            display_error_message();
            break;
    }
}

void MainWidget::display_error_message()
{
    QMessageBox::warning(this, QString("Media Error"), music_player->errorString());
}

void MainWidget::handle_cursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if (status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void MainWidget::update_serial_port_list()
{
    ports = serial::list_ports();
    for (auto &port : ports)
    {
        if (port.description.find("Serial Port") != std::string::npos)
        {
            ui.xbee_port_combobox->insertItem(0, QString::fromStdString(port.port));
        }
    }
}

void MainWidget::any_event()
{
    blink_midi_indicator();
}

void MainWidget::blink_midi_indicator()
{
    ui.midi_indicator_button->setEnabled(true);
    QTimer::singleShot(50, ui.midi_indicator_button, [&]()
    {
        ui.midi_indicator_button->setEnabled(false);
    });
}

void MainWidget::track_range_changed(int const min, int const max)
{
    ui.track_spinbox->setRange(min, max);
}

void MainWidget::event_count_changed(int const event_count)
{
    auto const str = QString("%1 Events").arg(event_count);
    ui.event_count_label->setText(str);
}
