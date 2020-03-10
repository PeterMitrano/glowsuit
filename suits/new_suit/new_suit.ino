#include <XBee.h>
#include <SoftwareSerial.h>

#include "suit_1.h"

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


unsigned int time_offset = 0;

void flashLed(unsigned int on_time);

void setup() {
  delay(1000);

  pinMode(led_pin, OUTPUT);
  for (auto const pin : channel_to_pin) {
    pinMode(pin, OUTPUT);
  }

  Serial.begin(57600);
  nss.begin(9600);

  for (auto i{0u}; i < suit_number; ++i) {
    flashLed(100);
  }

  xbee.setSerial(Serial);

  // Synchronize time
  // wait for a message from the master, which should be 2 bytes, and just contain the current global time
  xbee.readPacket();
  xbee.getResponse().getRx16Response(rx16);
  uint16_t hb = *rx16.getData();
  uint16_t lb = *rx16.getData();
  uint16_t current_global_time = hb << 8 + lb;
  time_offset = current_global_time - millis();
  flashLed(1000);

  // respond with an acknowledgement message containing this suit's number (1 byte)
  uint8_t payload[] = { suit_number };
  Tx16Request tx = Tx16Request(0x00, payload, sizeof(payload));
  xbee.send(tx);
  flashLed(1000);

  // now wait until the global start time (30 seconds) is reached
  while (millis() + time_offset <= 30000);

  // ITS GO TIME!!!
}

unsigned int choreo_idx = 0u;
void loop() {
  auto const global_now_cs = (millis() + time_offset) / 10;
  auto const next_onset_cs = pgm_read_word_near(choreo + (choreo_idx * 3));

  if (global_now_cs >= next_onset_cs)
  {
    // display the current state
    auto const current_state = pgm_read_byte_near(choreo + (choreo_idx * 3 + 2));
    size_t bit_idx = 0;
    for (auto const pin : channel_to_pin) {
      auto const channel_on = static_cast<bool>((current_state >> bit_idx) & 0x01);
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
  delay(100);
}
