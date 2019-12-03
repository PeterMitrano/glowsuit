#include <XBee.h>
#include <SoftwareSerial.h>

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

uint8_t suit_number = 0;

constexpr int ni_req_buff_len = 8;
constexpr uint8_t ni_req_buff[ni_req_buff_len] = {126, 0, 4, 9, 1, 77, 89, 79};
constexpr int ni_res_buff_len = 11;

uint8_t get_suit_number();
void flashLed(unsigned int on_time);

void setup() {
  pinMode(led_pin, OUTPUT);
  for (auto const pin : channel_to_pin) {
    pinMode(pin, OUTPUT);
  }

  Serial.begin(57600);
  nss.begin(9600);

  xbee.setSerial(Serial);

  suit_number = get_suit_number();
  nss.print("Suit number: ");
  nss.println(suit_number);

  flashLed(500);
}

void loop() {
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
      xbee.getResponse().getRx16Response(rx16);

      uint8_t command = rx16.getData(0);
      uint8_t dest_channel = rx16.getData(1);
      if (command == 128) { // off
        digitalWrite(channel_to_pin[dest_channel], LOW);
      }
      else if (command == 144) { // on
        digitalWrite(channel_to_pin[dest_channel], HIGH);
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
  const int suit_number = ni_res_buff[ni_res_buff_len - 2] << 8 + ni_res_buff[ni_res_buff_len - 1];
  return suit_number;
}

void flashLed(unsigned int on_time) {
  // just for debugging
  digitalWrite(led_pin, HIGH);
  delay(on_time);
  digitalWrite(led_pin, LOW);
}
