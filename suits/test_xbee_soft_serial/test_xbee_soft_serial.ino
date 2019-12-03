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
  pinMode(led_pin, OUTPUT);
  
  Serial.begin(57600);
  nss.begin(9600);

  
  xbee.setSerial(Serial);

  
  Serial.println("setup");
  nss.println("setup");
  flashLed(500);

  
  return;
  
  // TODO: get the suit number from the xbee Node ID
  AtCommandRequest niRequest = AtCommandRequest(node_id_command);
  AtCommandResponse niResponse = AtCommandResponse();

  // send the command
  xbee.send(niRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(1000)) {
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
      nss.println(xbee.getResponse().getApiId(), HEX);
    }   
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      nss.print("Error reading packet.  Error code: ");  
      nss.println(xbee.getResponse().getErrorCode());
    } 
    else {
      nss.println("No response from radio");  
    }
  }
}

void loop() {
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
      xbee.getResponse().getRx16Response(rx16);
      
      uint8_t command = rx16.getData(0);
      uint8_t pitch = rx16.getData(1);
      const uint8_t dest_suit_number = pitch / num_channels;
      const uint8_t dest_channel = pitch % num_channels;
      nss.println(command);
      nss.println(pitch);
      nss.println(dest_suit_number);
      nss.println(dest_channel);
      if (command == 128) { // off 
        digitalWrite(dest_channel, LOW);
      }
      else if (command == 144) { // on
        digitalWrite(dest_channel, HIGH);
      }     
      
    } else {
      flashLed(50);
      nss.println("not 16 rx");
    }
  } else if (xbee.getResponse().isError()) {
    nss.println("error");
  }
}

void flashLed(unsigned int on_time) {
  // just for debugging
  digitalWrite(led_pin, HIGH);
  delay(on_time);
  digitalWrite(led_pin, LOW);
}
