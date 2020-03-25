#pragma once

#include <QObject>
#include <mutex>

#include <common.h>

class SuitWorker : public QObject
{
Q_OBJECT

public:
    explicit SuitWorker(unsigned int suit_idx, QObject *parent = nullptr);

    void digitalWrite(unsigned long pin, bool on);

    DataAndLength readPacket(bool blocking);

    unsigned long millis();

    uint8_t pgm_read_byte_near(uint8_t const *address);

    uint16_t pgm_read_word_near(uint8_t const *address);

public slots:

    void start();

    void receive_time(std::vector<uint8_t> const &data, unsigned long size);

signals:

    void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void my_finished();

public:

private:
    unsigned int suit_idx;
    DataAndLength latest_data;
    std::mutex new_data_mutex;
    bool new_data{false};
    std::chrono::high_resolution_clock::time_point start_time;
};

