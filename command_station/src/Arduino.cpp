#include <chrono>
#include <include/mock_arduino/Arduino.h>
#include <suit_dispatcher.h>

void delay(unsigned long const dt_ms)
{
    usleep(dt_ms * 1000UL);
}

void digitalWrite(unsigned long pin, bool on)
{
    SuitDispatcher::digitalWrite(pin, on);
}

unsigned long millis()
{
    return SuitDispatcher::millis();
}

uint8_t pgm_read_byte_near(uint8_t const *address)
{
    SuitDispatcher::pgm_read_byte_near(address);
}

uint16_t pgm_read_word_near(uint8_t const *address)
{
    SuitDispatcher::pgm_read_word_near(address);
}

void pinMode(int, int)
{}

void MockSerial::begin(int)
{}

MockSerial Serial;
