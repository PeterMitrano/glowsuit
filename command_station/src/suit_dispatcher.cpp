#include <iostream>

#include <QThread>

#include <suit_dispatcher.h>

std::unordered_map<int, std::shared_ptr<SuitWorker>> SuitDispatcher::suit_map;
QThreadStorage<int> SuitDispatcher::suit_thread_storage;

void SuitDispatcher::registerSuit(unsigned int const suit_idx, std::shared_ptr<SuitWorker> suit)
{
    suit_map.emplace(suit_idx, suit);
}

void SuitDispatcher::digitalWrite(unsigned long const pin, bool on)
{
    auto const suit_idx = suit_thread_storage.localData();
    auto const suit_worker = suit_map.at(suit_idx);
    suit_worker->digitalWrite(pin, on);
}

void SuitDispatcher::clear()
{
    suit_map.clear();
}

uint8_t SuitDispatcher::pgm_read_byte_near(uint8_t const *address)
{
    auto const suit_idx = suit_thread_storage.localData();
    auto const suit_worker = suit_map.at(suit_idx);
    return suit_worker->pgm_read_byte_near(address);
}

uint16_t SuitDispatcher::pgm_read_word_near(uint8_t const *address)
{
    auto const suit_idx = suit_thread_storage.localData();
    auto const suit_worker = suit_map.at(suit_idx);
    return suit_worker->pgm_read_word_near(address);
}

DataAndLength SuitDispatcher::readPacket(bool blocking)
{
    auto const suit_idx = suit_thread_storage.localData();
    auto const suit_worker = suit_map.at(suit_idx);
    return suit_worker->readPacket(blocking);
}

unsigned long SuitDispatcher::millis()
{
    auto const suit_idx = suit_thread_storage.localData();
    auto const suit_worker = suit_map.at(suit_idx);
    return suit_worker->millis();
}
