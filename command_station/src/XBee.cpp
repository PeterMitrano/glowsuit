#include <XBee.h>
#include <suit_dispatcher.h>

Rx16Response::Rx16Response(Data const &data_and_length)
{
    // parse the packet
    auto packet = data_to_packet(data_and_length);
    latest_payload = packet->data;
}

uint8_t const *Rx16Response::getData() const
{
    return latest_payload.data();
}

uint16_t Rx16Response::getDataLength() const
{
    return latest_payload.size();
}

bool XBeeResponse::isAvailable() const
{
    return latest_response.getDataLength() > 0;
}

int XBeeResponse::getApiId() const
{
    return RX_16_RESPONSE;
}

void XBeeResponse::getRx16Response(Rx16Response &response)
{
    // copy
    response = {latest_response};
    // now invalidate/delete latest response
    latest_response = Rx16Response{};
}

XBeeResponse::XBeeResponse(Data const &data_and_length) : latest_response(data_and_length)
{}

void XBee::setSerial(MockSerial) const
{}

void XBee::readPacketUntilAvailable()
{
    auto const latest_data = SuitDispatcher::readPacket(true);
    latest_response = XBeeResponse{latest_data};
}

void XBee::readPacket()
{
    auto const latest_data = SuitDispatcher::readPacket(false);
    latest_response = XBeeResponse{latest_data};
}

XBeeResponse XBee::getResponse() const
{
    return latest_response;
}
