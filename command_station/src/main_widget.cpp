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

    // these are pointers because you can't use "this" in an the constructor initializer list
    visualizer = new Visualizer(this);
    num_channels = visualizer->load_suit();
    ui.visualizer_group->layout()->addWidget(visualizer);
    live_midi_worker = new LiveMidiWorker(num_channels, this);
    music_player = new QMediaPlayer(this);
    music_player->setNotifyInterval(100);

    connect(music_player, &QMediaPlayer::mediaStatusChanged, this, &MainWidget::status_changed);
    connect(music_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this,
            &MainWidget::display_error_message);

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
    connect(ui.num_suits_spinbox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWidget::num_suits_changed);
    connect(ui.start_button, &QPushButton::clicked, this, &MainWidget::start_clicked);
    connect(ui.abort_button, &QPushButton::clicked, this, &MainWidget::abort_clicked);

    // start a thread for receiving MIDI
    live_midi_worker->moveToThread(&live_midi_thread);
    connect(&live_midi_thread, &QThread::started, live_midi_worker, &LiveMidiWorker::listen_for_midi);
    connect(live_midi_worker, &LiveMidiWorker::my_finished, &live_midi_thread, &QThread::quit);
    connect(live_midi_worker, &LiveMidiWorker::midi_event, visualizer, &Visualizer::on_live_midi_event);
    connect(live_midi_worker, &LiveMidiWorker::any_event, this, &MainWidget::any_event);
    live_midi_thread.start();

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
}

MainWidget::~MainWidget()
{
    music_player->stop();

    live_midi_thread.requestInterruption();
    live_midi_thread.quit();
    live_midi_thread.wait();

    // delete pointers that have no parents
    delete live_midi_worker;
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
    xbee_serial = new serial::Serial(port_name, baud_rate, serial::Timeout::simpleTimeout(1000));

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
    settings->setValue("gui/num_suits", ui.num_suits_spinbox->value());
    settings->setValue("gui/controls_hidden", controls_hidden);
    settings->setValue("files/music", music_filename);
    settings->setValue("files/midi", midi_filename);
    settings->setValue("gui/xbee_port", previously_selected_port_name);
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
    ui.num_suits_spinbox->setValue(settings->value("gui/num_suits").toInt());
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

void MainWidget::num_suits_changed(int value)
{
    num_suits = value;
    for (auto *checkbox : suit_status_checkboxes)
    {
        ui.suit_checkbox_layout->removeWidget(checkbox);
        checkbox->deleteLater();
    }
    suit_status_checkboxes.clear();

    for (auto i = 0; i < value; ++i)
    {
        auto *checkbox = new QCheckBox(this);
        checkbox->setText(QString::number(i + 1));
        checkbox->setEnabled(false);
        suit_status_checkboxes.emplace_back(checkbox);
        ui.suit_checkbox_layout->addWidget(checkbox);
    }
}

void MainWidget::abort_clicked()
{
    for (auto *checkbox : suit_status_checkboxes)
    {
        checkbox->setChecked(true);
    }

    music_player->stop();
    aborted = true;
    if (sync_xbees_thread.joinable())
    {
        sync_xbees_thread.join();
    }

    for (auto *checkbox : suit_status_checkboxes)
    {
        checkbox->setChecked(false);
    }
}

void MainWidget::start_clicked()
{
    // repeatedly send a message containing the current time
    if (not xbee_serial)
    {
        return;
    }

    // make sure the thread isn't still running
    if (sync_xbees_thread.joinable())
    {
        sync_xbees_thread.join();
    }

    auto func = [&]()
    {
        // this right here defined the entire global song time for everyone
        auto const start_time = std::chrono::high_resolution_clock::now().time_since_epoch();
        auto const start_ms = std::chrono::duration_cast<std::chrono::milliseconds>(start_time).count();

        bool done = false;
        std::vector<uint8_t> suits_ready;
        constexpr auto tx_status_message_size = 7u;
        std::vector<uint8_t> tx_status_message_buffer;
        tx_status_message_buffer.resize(tx_status_message_size);
        constexpr auto ready_message_size = 10u;
        std::vector<uint8_t> ready_message_buffer;
        ready_message_buffer.resize(ready_message_size);
        aborted = false;
        while (!aborted)
        {
            auto const now_time = std::chrono::high_resolution_clock::now().time_since_epoch();
            auto const now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_time).count();
            auto const dt_ms = static_cast<uint32_t>(now_ms - start_ms);

            sendTime(dt_ms);

            QThread::msleep(100);

            if (dt_ms > 5000)
            {
                break;
            }
        }

        if (aborted)
        {
            return;
        }

        std::cout << "Waiting to start...\n";

        // Everyone is ready! wait until 30 seconds then start the music
        constexpr auto startup_delay_ms = 15000;
        while (true)
        {
            auto const now_time = std::chrono::high_resolution_clock::now().time_since_epoch();
            auto const now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_time).count();
            auto const dt_ms = static_cast<uint32_t>(now_ms - start_ms);
            if (dt_ms <= startup_delay_ms - 500)
            {
                std::cout << dt_ms << "\n";
                QThread::sleep(1);
            }
            if (dt_ms >= startup_delay_ms)
            {
                break;
            }
        }

        // Start the music!
        music_player->play();

//        while (true)
//        {
//            auto const now_time = std::chrono::high_resolution_clock::now().time_since_epoch();
//            auto const now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_time).count();
//            auto const dt_ms = static_cast<uint32_t>(now_ms - start_ms);
//
//            QThread::sleep(1);
//
//            sendTime(dt_ms);
//            std::cout << dt_ms << '\n';
//
//            // TODO: make this based on song length
//            if (dt_ms >= 5 * 60 * 1000)
//            {
//                break;
//            }
//        }
    };

    sync_xbees_thread = std::thread(func);
}

void MainWidget::sendTime(long const dt_ms)
{
    auto const time_bytes = to_bytes<uint32_t>(dt_ms);

    try
    {
        auto const[packet, size] = make_packet(time_bytes);
        xbee_serial->write(packet.data(), size);
    } catch (serial::SerialException &)
    {
        std::cerr << "time message failed to send! \n";
    }
}
