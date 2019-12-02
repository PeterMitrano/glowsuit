#include <XBee.h>
#include <SoftwareSerial.h>

// Define SoftSerial TX/RX pins
uint8_t ssRX = A2;
uint8_t ssTX = A3;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial nss(ssRX, ssTX);

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
Rx64Response rx64 = Rx64Response();

constexpr int led_pin = 13;
constexpr int num_channels = 8;

constexpr  int channel_to_pin[8] = {
  32,  // 0
  1,   // 1
  2,   // 2
  9,   // 3
  10,  // 4
  11,  // 5
  12,  // 6
  13,  // 7  
};

uint8_t suit_number = 0;


// node information
uint8_t node_id_command[] = {'N','I'};

void flashLed(unsigned int on_time);

void setup() {
  Serial.begin(9600);
  xbee.setSerial(Serial);
  pinMode(led_pin, OUTPUT);

  // TODO: get the suit number from the xbee Node ID
  AtCommandRequest niRequest = AtCommandRequest(node_id_command);
  AtCommandResponse niResponse = AtCommandResponse();

  // send the command
  xbee.send(niRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(niResponse);

      if (niResponse.isOk()) {
        nss.print("Command [");
        nss.print(niResponse.getCommand()[0]);
        nss.print(niResponse.getCommand()[1]);
        nss.println("] was successful!");

        if (niResponse.getValueLength() > 0) {
          nss.print("Command value length is ");
          nss.println(niResponse.getValueLength(), DEC);

          nss.print("Command value: ");
          suit_number = *niResponse.getValue();
          for (int i = 0; i < niResponse.getValueLength(); i++) {
            nss.print(niResponse.getValue()[i], HEX);
            nss.print(" ");
          }

          nss.println("");
        }
      } 
      else {
        nss.print("Command return error code: ");
        nss.println(niResponse.getStatus(), HEX);
      }
    } else {
      nss.print("Expected AT response but got ");
      nss.print(xbee.getResponse().getApiId(), HEX);
    }   
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      nss.print("Error reading packet.  Error code: ");  
      nss.println(xbee.getResponse().getErrorCode());
    } 
    else {
      nss.print("No response from radio");  
    }
  }
}

void loop() {
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == RX_64_RESPONSE) {
      xbee.getResponse().getRx64Response(rx64);
      
      uint8_t command = rx64.getData(0);
      uint8_t pitch = rx64.getData(1);
      const uint8_t dest_suit_number = pitch / num_channels;
      const uint8_t dest_channel = pitch % num_channels;

      if (command == 128) { // off 
        digitalWrite(dest_channel, LOW);
      }
      else if (command == 144) { // on
        digitalWrite(dest_channel, HIGH);
      }     
      flashLed(10);
    } else {
      flashLed(50);
    }
  } else if (xbee.getResponse().isError()) {
    flashLed(25);
  }
}

void flashLed(unsigned int on_time) {
  // just for debugging
  digitalWrite(led_pin, HIGH);
  delay(on_time);
  digitalWrite(led_pin, LOW);
}
