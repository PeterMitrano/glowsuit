#include <Arduino.h>
#include <ArduinoSTL.h>
#include <XBee.h>

#include "suit_common.h"
#include "suit_program.h"

int32_t millis_signed()
{
    return static_cast<int32_t>(millis());
}


SuitProgram::SuitProgram(uint8_t const *choreo, uint16_t const num_events) : choreo(choreo), num_events(num_events)
{
}

void SuitProgram::setup()
{
    delay(1000);

    pinMode(led_pin, OUTPUT);
    for (auto const pin : channel_to_pin)
    {
        pinMode(pin, OUTPUT);
    }

    Serial.begin(57600);

    for (auto i{0u}; i < suit_number; ++i)
    {
        flashLed(200);
    }

    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, HIGH);
    }

    xbee.setSerial(Serial);

    while (true)
    {
        xbee.readPacketUntilAvailable();
        Rx16Response response;
        xbee.getResponse().getRx16Response(response);
        auto const suit_command = response_to_suit_command(response.getData(), response.getDataLength());
        if (suit_command.type == CommandType::Time)
        {
            update_time(suit_command);
            break;
        }
    }

    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, LOW);
    }

    // now wait until the global start time (N seconds) is reached
    while ((millis_signed() - time_offset_ms) <= startup_delay_ms)
    {
        // read packets so they dont just size in our buffer
        xbee.readPacket();
    }
}

void SuitProgram::loop()
{
    int32_t const now_ms = millis_signed() - time_offset_ms;
    int32_t const choreo_time_cs = (now_ms - startup_delay_ms) / 10;

    // Find the next upcoming event and set choreo_idx
    uint16_t choreo_idx = num_events;
    for (auto j = 0u; j < num_events; ++j)
    {
        auto const onset_cs = pgm_read_word_near(choreo + (j * 3));
        if (onset_cs >= choreo_time_cs)
        {
            choreo_idx = j;
            break;
        }
    }

    if (choreo_idx >= num_events)
    {
        all_off();
        return;
    };

    if (not paused)
    {
        auto const next_onset_cs = pgm_read_word_near(choreo + (choreo_idx * 3));
        if (choreo_time_cs >= next_onset_cs)
        {
            // display the current state
            auto const current_state = pgm_read_byte_near(choreo + (choreo_idx * 3 + 2));
            size_t bit_idx = 0;
            for (auto const pin : channel_to_pin)
            {
                auto const channel_on = (bool) ((current_state >> bit_idx) & 0x01);
                digitalWrite(pin, channel_on ? HIGH : LOW);
                ++bit_idx;
            }
        }
    }

    handle_packets();
}

void SuitProgram::handle_packets()
{
    xbee.readPacket();
    auto response = xbee.getResponse();
    auto const available = response.isAvailable();
    auto const api_id = response.getApiId();
    if (available and api_id == RX_16_RESPONSE)
    {
        Rx16Response rx16;
        response.getRx16Response(rx16);

        auto const suit_command = response_to_suit_command(rx16.getData(), rx16.getDataLength());
        switch (suit_command.type)
        {
            case CommandType::Time:
                update_time(suit_command);
                break;
            case CommandType::Pause:
                all_off();
                paused = true;
                break;
            case CommandType::Resume:
                all_off();
                paused = false;
                std::cout << "resume\n";
                break;
            case CommandType::State:
                break;
            default:
                break;
        }
    }
}

void SuitProgram::update_time(SuitCommand const &cmd)
{
    auto const current_global_time_ms = from_bytes<int32_t>(cmd.data);
    auto const now = millis_signed();
    time_offset_ms = now - current_global_time_ms - transmission_delay_ms;
}

void SuitProgram::all_off()
{
    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, LOW);
    }
}

void SuitProgram::all_on()
{
    for (auto const pin : channel_to_pin)
    {
        digitalWrite(pin, HIGH);
    }
}
