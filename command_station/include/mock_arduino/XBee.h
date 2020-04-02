#pragma once

#include <cstdint>
#include <vector>

#include <Arduino.h>
#include <common.h>

constexpr auto RX_16_RESPONSE{1};

class Rx16Response
{
public:
    Rx16Response() = default;

    explicit Rx16Response(Data const &data_and_length);

    [[nodiscard]] uint8_t const *getData() const;

    [[nodiscard]] uint16_t getDataLength() const;

private:
    std::vector<uint8_t> latest_payload;
};

class XBeeResponse
{
public:
    XBeeResponse() = default;

    explicit XBeeResponse(Data const &data_and_length);

    [[nodiscard]] bool isAvailable() const;

    [[nodiscard]] int getApiId() const;

    // Calling this will clear the latest response
    void getRx16Response(Rx16Response &);

private:
    Rx16Response latest_response;
};

class XBee
{
public:
    void setSerial(MockSerial) const;

    void readPacketUntilAvailable();

    void readPacket();

    [[nodiscard]] XBeeResponse getResponse() const;

private:
    XBeeResponse latest_response;
};
