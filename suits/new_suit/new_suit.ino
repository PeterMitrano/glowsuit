#include <XBee.h>
#include <SoftwareSerial.h>

#include "suit_2.h"

// Define SoftSerial TX/RX pins
uint8_t ssRX = A4;
uint8_t ssTX = A2;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial nss(ssRX, ssTX);

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
Rx16Response rx16 = Rx16Response();

constexpr int led_pin = 13;
constexpr int num_channels = 8;

constexpr  int channel_to_pin[8] = {
  2,  // 0
  3,   // 1
  4,   // 2
  5,   // 3
  6,  // 4
  7,  // 5
  8,  // 6
  9,  // 7
};


long time_offset_ms = 0;

void flashLed(unsigned int on_time);
constexpr long startup_delay = 15000;
constexpr long transmission_delay_ms = 60;

long millis_signed()
{
  return static_cast<long>(millis());
}

void setup() {
  delay(1000);

  pinMode(led_pin, OUTPUT);
  for (auto const pin : channel_to_pin) {
    pinMode(pin, OUTPUT);
  }

  Serial.begin(57600);
  nss.begin(9600);

  for (auto i{0u}; i < suit_number; ++i) {
    flashLed(200);
  }

  xbee.setSerial(Serial);

  // Synchronize time
  // wait for a message from the master, which should be 2 bytes
  // and just contain the current global time
  for (auto const pin : channel_to_pin) {
    digitalWrite(pin, HIGH);
  }

  while (true)
  {
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
      xbee.getResponse().getRx16Response(rx16);
      uint16_t hb = *rx16.getData();
      uint16_t lb = *rx16.getData();
      long current_global_time_ms = hb << 8 + lb;
      time_offset_ms = current_global_time_ms - millis_signed() + transmission_delay_ms;
      break;
    }
  }

  for (auto const pin : channel_to_pin) {
    digitalWrite(pin, LOW);
  }

  // now wait until the global start time (N seconds) is reached
  while ((millis_signed() + time_offset_ms) <= startup_delay)
  {

  }

  // ITS GO TIME!!!
}

uint16_t choreo_idx = 0;
void loop() {
  if (choreo_idx >= num_events - 1) {
    for (auto const pin : channel_to_pin) {
      digitalWrite(pin, LOW);
    }
    return;
  };

  // Check if a new time-sync is available
  //xbee.readPacket();
  //if (xbee.getResponse().isAvailable()) {
    //xbee.getResponse().getRx16Response(rx16);
    //uint16_t hb = *rx16.getData();
    //uint16_t lb = *rx16.getData();
    //long current_global_time_ms = hb << 8 + lb;
    //time_offset_ms = current_global_time_ms - millis_signed();
    //flashLed(100);
  //}

  auto const global_now_cs = (millis_signed() + time_offset_ms- startup_delay) / 10;
  auto const next_onset_cs = pgm_read_word_near(choreo + (choreo_idx * 3));

  if (global_now_cs >= next_onset_cs)
  {
    // display the current state
    auto const current_state = pgm_read_byte_near(choreo + (choreo_idx * 3 + 2));
    size_t bit_idx = 0;
    for (auto const pin : channel_to_pin) {
      auto const channel_on = (bool)((current_state >> bit_idx) & 0x01);
      digitalWrite(pin, channel_on ? HIGH : LOW);
      ++bit_idx;
    }

    // move on to next event
    ++choreo_idx;
  }
}


void flashLed(unsigned int on_time) {
  // just for debugging
  digitalWrite(led_pin, HIGH);
  delay(on_time);
  digitalWrite(led_pin, LOW);
  delay(200);
}
