#include <chrono>

#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringBuilder>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QTime>

#include <common.h>
#include <main_widget.h>

MainWidget::MainWidget(QWidget *parent)
        : QWidget(parent)
{
    ui.setupUi(this);

    ui.play_button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui.stop_button->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    connect(ui.play_button, &QAbstractButton::clicked, this, &MainWidget::play_pause_clicked);
    connect(ui.stop_button, &QAbstractButton::clicked, this, &MainWidget::stop);
    connect(ui.start_with_countdown_button, &QPushButton::clicked, this, &MainWidget::start_with_countdown);


    // these are pointers because you can't use "this" in an the constructor initializer list
    visualizer = new Visualizer(this);
    num_channels = visualizer->load_suit();
    ui.visualizer_group->layout()->addWidget(visualizer);
    live_midi_worker = new LiveMidiWorker(num_channels, this);
    music_player = new QMediaPlayer(this);
    music_player->setNotifyInterval(500);

    connect(music_player, &QMediaPlayer::mediaStatusChanged, this, &MainWidget::status_changed);
    connect(music_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this,
            &MainWidget::display_error_message);
    ui.player_slider->setRange(0, static_cast<int>(music_player->duration()));

    connect(ui.player_slider, &QSlider::sliderMoved, this, &MainWidget::seek);
    connect(music_player, &QMediaPlayer::durationChanged, this, &MainWidget::duration_changed);
    connect(music_player, &QMediaPlayer::positionChanged, this, &MainWidget::position_changed);
    connect(this, &MainWidget::play, music_player, &QMediaPlayer::play);
    connect(this, &MainWidget::pause, music_player, &QMediaPlayer::pause);
    connect(this, &MainWidget::stop, music_player, &QMediaPlayer::stop);
    connect(music_player, &QMediaPlayer::stateChanged, this, &MainWidget::set_state);

    connect(ui.front_button, &QPushButton::clicked, visualizer, &Visualizer::front_status_clicked);
    connect(ui.back_button, &QPushButton::clicked, visualizer, &Visualizer::back_status_clicked);
    connect(ui.scale_spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), visualizer,
            &Visualizer::viz_scale_changed);
    connect(ui.select_music_file_button, &QPushButton::clicked, this, &MainWidget::music_file_button_clicked);
    connect(ui.all_on_button, &QPushButton::clicked, this, &MainWidget::all_on_clicked);
    connect(ui.all_off_button, &QPushButton::clicked, this, &MainWidget::all_off_clicked);
    connect(ui.live_checkbox, &QCheckBox::stateChanged, this, &MainWidget::live_midi_changed);
    connect(ui.octave_spinbox, qOverload<int>(&QSpinBox::valueChanged), live_midi_worker,
            &LiveMidiWorker::octave_spinbox_changed);
    connect(ui.user_visualizer_checkbox, &QCheckBox::stateChanged, visualizer, &Visualizer::use_visualizer_checked);

    set_state(music_player->state());

    // start a thread for receiving MIDI
    live_midi_worker->moveToThread(&live_midi_thread);
    connect(&live_midi_thread, &QThread::started, live_midi_worker, &LiveMidiWorker::listen_for_midi);
    connect(live_midi_worker, &LiveMidiWorker::my_finished, &live_midi_thread, &QThread::quit);
    connect(live_midi_worker, &LiveMidiWorker::midi_event, visualizer, &Visualizer::on_live_midi_event);
    connect(live_midi_worker, &LiveMidiWorker::any_event, this, &MainWidget::any_event);
    live_midi_thread.start();

    for (auto suit_idx{0u}; suit_idx < num_suits; ++suit_idx)
    {
        auto *suit_worker = new SuitWorker(suit_idx);
        auto *suit_thread = new QThread();
        suit_worker->moveToThread(suit_thread);
        connect(suit_thread, &QThread::started, suit_worker, &SuitWorker::start);
        connect(suit_worker, &SuitWorker::my_finished, suit_thread, &QThread::quit);
        connect(suit_worker, &SuitWorker::midi_event, visualizer, &Visualizer::on_midi_file_event);
        connect(this, &MainWidget::send_time, suit_worker, &SuitWorker::receive_time);

        suit_workers.push_back(suit_worker);
        suit_threads.push_back(suit_thread);
    }

    connect(this, &MainWidget::gui_midi_event, visualizer, &Visualizer::generic_on_midi_event);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWidget::update_serial_port_list);
    timer->start(4000);

    connect(ui.xbee_port_combobox, qOverload<int>(&QComboBox::activated), this,
            &MainWidget::xbee_port_changed);

    // do this before restoring settings so we can restore the XBee port, if it exists
    update_serial_port_list();
    restore_settings();

    if (!music_player->isAvailable())
    {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                "Please check the media service plugins are installed."));

        ui.midi_file_group->setEnabled(false);
    }

    // start fake suit programs
}

MainWidget::~MainWidget()
{
    // stop music
    music_player->stop();

    // stop all threads
    live_midi_thread.requestInterruption();
    live_midi_thread.quit();
    live_midi_thread.wait();

    for (auto *suit_thread: suit_threads)
    {
        suit_thread->requestInterruption();
        suit_thread->quit();
        suit_thread->wait();
    }

    // delete pointers that have no parents
    delete live_midi_worker;
    for (auto *suit_worker : suit_workers)
    {
        delete suit_worker;
    }
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
    auto const port_name = ui.xbee_port_combobox->itemText(index).toStdString();
    xbee_serial = new serial::Serial(port_name, baud_rate, serial::Timeout::simpleTimeout(50));

    // TODO: is this an error? possible data race
    live_midi_worker->xbee_serial = xbee_serial;
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
    settings->setValue("gui/controls_hidden", controls_hidden);
    settings->setValue("files/music", music_filename);
    settings->setValue("files/midi", midi_filename);
    settings->setValue("gui/xbee_port", previously_selected_port_name);
    settings->setValue("gui/use_visualizer", visualizer->use_visualizer);
}

void MainWidget::restore_settings()
{
    QCoreApplication::setOrganizationName("Photonix");
    QCoreApplication::setApplicationName("Glowsuit");
    settings = new QSettings();

    music_filename = settings->value("files/music").toString();
    ui.music_filename_label->setText(music_filename);

    music_player->setMedia(QUrl::fromLocalFile(music_filename));

    ui.octave_spinbox->setValue(settings->value("gui/octave").toInt());
    ui.scale_spinbox->setValue(settings->value("gui/scale").toDouble());
    ui.live_checkbox->setChecked(settings->value("gui/live_checkbox").toBool());
    ui.user_visualizer_checkbox->setChecked(settings->value("gui/use_visualizer").toBool());
    controls_hidden = settings->value("gui/controls_hidden").toBool();
    ui.controls_widget->setVisible(!controls_hidden);

    emit visualizer->viz_scale_changed(ui.scale_spinbox->value());

    ui.midi_file_group->setEnabled(!ui.live_checkbox->isChecked());

    previously_selected_port_name = settings->value("gui/xbee_port").toString();
    for (auto i{0u}; i < ui.xbee_port_combobox->count(); ++i)
    {
        if (previously_selected_port_name == ui.xbee_port_combobox->itemText(i))
        {
            ui.xbee_port_combobox->setCurrentIndex(i);
            emit xbee_port_changed(i);
        }
    }
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
                if (startup_sync_thread.joinable())
                {
                    startup_sync_thread.join();
                }
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

void MainWidget::seek(int milliseconds)
{
    music_player->setPosition(milliseconds);
}

void MainWidget::duration_changed(qint64 duration)
{
    this->song_duration_ms = duration;
    ui.player_slider->setMaximum(this->song_duration_ms);
}

void MainWidget::position_changed(qint64 progress)
{
    if (!ui.player_slider->isSliderDown())
        ui.player_slider->setValue(progress);

    update_duration_info(progress);

    sendTime(progress + start_delay_ms_);
}

void MainWidget::update_duration_info(qint64 time_ms)
{
    QString time_str;
    if (time_ms || song_duration_ms)
    {
        QTime current_time(static_cast<int>(time_ms / 3600000) % 60,
                           static_cast<int>(time_ms / 60000) % 60,
                           static_cast<int>(time_ms / 1000) % 60,
                           static_cast<int>(time_ms) % 1000);
        QTime total_time(static_cast<int>(song_duration_ms / 3600000) % 60,
                         static_cast<int>(song_duration_ms / 60000) % 60,
                         static_cast<int>(song_duration_ms / 1000) % 60,
                         static_cast<int>(song_duration_ms) % 1000);
        QString format = "mm:ss";
        if (song_duration_ms > 3600000)
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
    if (status == QMediaPlayer::InvalidMedia)
    {
        display_error_message();
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
    previously_selected_port_name = ui.xbee_port_combobox->currentText();

    auto new_ports = serial::list_ports();
    // filter out ports that probably aren't serial
    auto probably_not_serial = [&](serial::PortInfo const &new_port)
    {
        return (new_port.description.find("Serial Port") == std::string::npos) &&
               (new_port.description.find("USB") == std::string::npos) &&
               (new_port.description.find("usbmodem") == std::string::npos);
    };
    new_ports.erase(std::remove_if(new_ports.begin(), new_ports.end(), probably_not_serial), new_ports.end());

    // update the list
    ui.xbee_port_combobox->clear();
    for (auto const &port : new_ports)
    {
        ui.xbee_port_combobox->insertItem(0, QString::fromStdString(port.port));
    }

    // re-select if the previously selected port still exists
    auto const previously_selected_item_idx = ui.xbee_port_combobox->findText(previously_selected_port_name);
    if (previously_selected_item_idx != -1)
    {
        ui.xbee_port_combobox->setCurrentIndex(previously_selected_item_idx);
    }

    // if only one device exists, consider it selected
    if (ui.xbee_port_combobox->count() == 1 && previously_selected_port_name != ui.xbee_port_combobox->itemText(0))
    {
        ui.xbee_port_combobox->setCurrentIndex(0);
        emit xbee_port_changed(0);
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

void MainWidget::all_on_clicked()
{
    // FIXME: right now the suits wont respond to this
    for (auto suit_idx{1u}; suit_idx <= num_suits; ++suit_idx)
    {
        for (auto channel_idx{0u}; channel_idx < num_channels; ++channel_idx)
        {
            emit gui_midi_event(suit_idx, midi_note_on, channel_idx);
        }
    }
    State all_on_data;
    all_on_data.data.fill(0xFF);
    auto const[packet, size] = make_packet(all_on_data);
    if (xbee_serial)
    {
        xbee_serial->write(packet.data(), size);
    }
}

void MainWidget::all_off_clicked()
{
    for (auto suit_idx{1u}; suit_idx <= num_suits; ++suit_idx)
    {
        for (auto channel_idx{0u}; channel_idx < num_channels; ++channel_idx)
        {
            emit gui_midi_event(suit_idx, midi_note_off, channel_idx);
        }
    }
    State all_off_data; // default constructor fills to zero
    auto const[packet, size] = make_packet(all_off_data);
    if (xbee_serial)
    {
        xbee_serial->write(packet.data(), size);
    }
}

void MainWidget::keyReleaseEvent(QKeyEvent *event)
{
    auto const key = event->key();
    if (key == Qt::Key_H)
    {
        controls_hidden = !controls_hidden;
        ui.controls_widget->setVisible(!controls_hidden);
    }

}

void MainWidget::start_with_countdown()
{
    // this right here defines the entire global song time for everyone
    auto const start_time = std::chrono::high_resolution_clock::now().time_since_epoch();
    start_ms_ = std::chrono::duration_cast<std::chrono::milliseconds>(start_time).count();

    auto func = [&]()
    {
        while (true)
        {
            auto const now_time = std::chrono::high_resolution_clock::now().time_since_epoch();
            auto const now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_time).count();
            auto const dt_ms = static_cast<uint32_t>(now_ms - start_ms_);

            sendTime(dt_ms);

            if (dt_ms >= start_delay_ms_)
            {
                break;
            }

            QThread::msleep(10);
        }

        // Start the music!
        music_player->play();

        // print any data we get back
//        auto packet = read_packet(xbee_serial);
//        if (packet and packet->command_id == RX_16)
//        {
//            for (auto const &byte : packet->data)
//            {
//                if (byte >= 32 and byte < 127)
//                {
//                    std::cout << byte;
//                }
//            }
//            std::cout << std::endl;
//        }
    };

    startup_sync_thread = std::thread(func);
}

void MainWidget::sendTime(qint64 song_time_ms)
{
    if (not xbee_serial)
    {
        return;
    }
    auto const time_bytes = to_bytes<uint32_t>(song_time_ms);

    try
    {
        auto const[tx_packet, size] = make_packet(time_bytes);
        xbee_serial->write(tx_packet.data(), size);
        emit send_time(tx_packet.data(), size);
    } catch (serial::SerialException &)
    {
        std::cerr << "time message failed to send! \n";
    }
}

