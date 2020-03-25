#pragma once

#include <unordered_map>
#include <memory>

#include <QThreadStorage>

#include <suit_worker.h>

class SuitDispatcher
{
public:
    static void registerSuit(unsigned int suit_idx, std::shared_ptr<SuitWorker> suit);

    static void digitalWrite(unsigned long pin, bool on);

    static void clear();

    static uint8_t pgm_read_byte_near(uint8_t const *);

    static uint16_t pgm_read_word_near(uint8_t const *);

    static DataAndLength readPacket(bool blocking = false);

    static unsigned long millis();

    // fields
    static QThreadStorage<int> suit_thread_storage;

private:
    static std::unordered_map<int, std::shared_ptr<SuitWorker>> suit_map;
};