#pragma once

#include <QObject>
#include <mutex>

#include <common.h>
#include <deque>

class SuitWorker : public QObject
{
Q_OBJECT

public:
    explicit SuitWorker(unsigned int suit_idx, QObject *parent = nullptr);

    void digitalWrite(unsigned long pin, bool on);

    Data readPacket(bool blocking);

    unsigned long millis();

    uint8_t pgm_read_byte_near(uint8_t const *address);

    uint16_t pgm_read_word_near(uint8_t const *address);

public slots:

    void start();

    void xbee_read(Data packet);

signals:

    void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void my_finished();

public:

private:
    unsigned int suit_idx;
    std::mutex new_packet_mutex;
    std::deque<Data> packet_queue;
    std::chrono::high_resolution_clock::time_point start_time;
    uint8_t num_events;
    std::vector<uint8_t> choreo;
};

