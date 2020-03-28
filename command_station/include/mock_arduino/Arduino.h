#pragma once

#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

constexpr auto HIGH{true};
constexpr auto LOW{false};
constexpr auto OUTPUT{0u};

// provided in arduino main code
void setup();
void loop();

// Mock these
void delay(unsigned long dt_ms);
void digitalWrite(unsigned long dt_ms, bool on);
unsigned long millis();
uint8_t pgm_read_byte_near(uint8_t const *);
uint16_t pgm_read_word_near(uint8_t const *);
void pinMode(int, int);

struct MockSerial
{
    void begin(int);
    void println(int x);
    void println(float x);
    void println(double x);
    void println(std::string const& s);
    void printf(const char* fmt, ...);
};

extern MockSerial Serial;
