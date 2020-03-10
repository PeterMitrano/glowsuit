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

constexpr int ni_req_buff_len = 8;
constexpr uint8_t ni_req_buff[ni_req_buff_len] = {0x7E, 0x00, 0x04, 0x08, 0x01, 0x4E, 0x49, 0x5F};
constexpr int ni_res_buff_len = 10;

uint8_t get_suit_number();
void flashLed(unsigned int on_time);

void setup() {
  delay(1000);
   
  pinMode(led_pin, OUTPUT);
  for (auto const pin : channel_to_pin) {
    pinMode(pin, OUTPUT);
  }

  Serial.begin(57600);
  nss.begin(9600);

//  for (byte k = 0; k < 5; k++) {
//    auto const displayInt = pgm_read_byte_near(choreo + k);
//    Serial.println(displayInt, HEX);
//  }
//  Serial.println();
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
  flashLed(1000);
  
  // respond with an acknowledgement message containing this suit's number (1 byte)
  uint8_t payload[] = { suit_number };
  Tx16Request tx = Tx16Request(0x00, payload, sizeof(payload));
  xbee.send(tx);

  flashLed(1000);
}

void loop() {
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
      xbee.getResponse().getRx16Response(rx16);

      uint8_t bit_pattern = rx16.getData(suit_number - 1);
      size_t bit_idx = 0;
      for (auto const pin : channel_to_pin) {
        auto const channel_on = static_cast<bool>((bit_pattern >> bit_idx) & 0x01);
        digitalWrite(pin, channel_on ? HIGH : LOW);
        ++bit_idx;
      }      
    } else {
      flashLed(50);
      nss.println("not 16 rx");
    }
  } else if (xbee.getResponse().isError()) {
    nss.println("error");
  }
}

uint8_t get_suit_number() {
  Serial.write(ni_req_buff, ni_req_buff_len);
  while (Serial.available() < ni_res_buff_len);
  uint8_t ni_res_buff[ni_res_buff_len];
  Serial.readBytes(ni_res_buff, ni_res_buff_len);
  // 3is the ascii character 0. This hack only works for numbers less than 10
  const int suit_number = static_cast<int>(ni_res_buff[ni_res_buff_len - 2]) - 48;
  return suit_number;
}

void flashLed(unsigned int on_time) {
  // just for debugging
  digitalWrite(led_pin, HIGH);
  delay(on_time);
  digitalWrite(led_pin, LOW);
  delay(100);
}
