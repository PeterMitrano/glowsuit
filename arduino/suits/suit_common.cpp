#include <Arduino.h>

#include <suit_common.h>

void flashLed(unsigned int on_time) {
  // just for debugging
  digitalWrite(led_pin, HIGH);
  delay(on_time);
  digitalWrite(led_pin, LOW);
  delay(200);
}

[[noreturn]] void infinite_error(unsigned int on_time) {
  while (true) {
    flashLed(on_time);
  }
}

[[maybe_unused]] SuitCommand response_to_suit_command(uint8_t const *data,
                                                      uint16_t const length) {
  if (length == 0) {
    infinite_error(100);
  }

  // check the first byte for the command type
  auto const command = static_cast<CommandType>(data[0]);
  if (command >= CommandType::End) {
    infinite_error(400);
  }
  std::vector<uint8_t> data_vec;
  for (auto i{1u}; i < length; ++i) {
    data_vec.push_back(data[i]);
  }
  return SuitCommand{command, data_vec};
}