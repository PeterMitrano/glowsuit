#include <iostream>

#include <QAbstractEventDispatcher>
#include <QThread>

#include <include/mock_arduino/Arduino.h>
#include <include/common.h>
#include <suit_common.h>
#include <suit_dispatcher.h>
#include <suit_worker.h>
#include <all_suits_choreo.h>

// It doesn't matter which one we include here
#include <suit_1.h>

SuitWorker::SuitWorker(unsigned int suit_idx, QObject *parent)
        : QObject(parent), suit_idx(suit_idx)
{
}

void SuitWorker::start()
{
    SuitDispatcher::suit_thread_storage.setLocalData(suit_idx);

    //TODO: support rebooting
    auto done{false};
    while (not done)
    {
        start_time = std::chrono::high_resolution_clock::now();
        setup();

        while (true)
        {
            // tiny sleep to prevent eating CPU
            QThread::msleep(1);

            // since this is an infinite loop, in order to receive Qt signals we need to call this manually
            thread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

            if (QThread::currentThread()->isInterruptionRequested())
            {
                done = true;
                break;
            }

            loop();
        }
    }
}

void SuitWorker::receive_time(std::vector<uint8_t> const &data, unsigned long const size)
{
    std::scoped_lock lock{new_data_mutex};
    latest_data = {data, size};
    new_data = true;
}

void SuitWorker::digitalWrite(unsigned long pin, bool on)
{
    switch (pin)
    {
        case led_pin:
        {
            break;
        }
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        {
            auto const command = on ? midi_note_on : midi_note_off;
            auto const channel_number = pin - 2;
            emit midi_event(suit_idx + 1, command, channel_number);
            break;
        }
        default:
            std::cout << "unimplemented pin " << pin << '\n';
            break;
    }
}

DataAndLength SuitWorker::readPacket(bool const blocking)
{
    if (blocking)
    {
        while (true)
        {
            // tiny sleep to prevent eating CPU
            QThread::msleep(1);

            thread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

            {
                std::scoped_lock lock{new_data_mutex};
                if (new_data)
                {
                    new_data = false;
                    // copy the latest data, and reset it
                    auto latest_data_copy{latest_data};
                    latest_data = {};
                    return latest_data_copy;
                }
            }
        }
    } else
    {

        if (new_data)
        {
            return latest_data;
        } else
        {
            return {};
        }
    }
}

unsigned long SuitWorker::millis()
{
    auto const now = std::chrono::high_resolution_clock::now();
    auto const duration = now - start_time;
    auto const millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

uint8_t SuitWorker::pgm_read_byte_near(uint8_t const *address)
{
    std::cout << address << " " << choreo[0] << '\n';
    return all_choreo[suit_idx][0];
}

uint16_t SuitWorker::pgm_read_word_near(uint8_t const *address)
{
    std::cout << address << " " << choreo[0] << '\n';
    return all_choreo[suit_idx][0];
}
