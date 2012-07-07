/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "sqmpacketprocessor.h"

SQMPacketProcessor::SQMPacketProcessor(QObject *parent) : QObject(parent)
{
    // protobuf lib validation
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}


void SQMPacketProcessor::newPacketReceived(DataPacket *packet)
{
    // parse packet
    Protocol::Packet protocolPacket;
    protocolPacket.ParseFromArray(packet->baRawPacketData->constData(), packet->intPacktLength);
    Protocol::Packet_PacketType packetType = protocolPacket.packettype();

    // handle packetType
    switch(packetType)
    {
        // login packet
        case Protocol::Packet_PacketType_LoginResponse :
        {
            Protocol::LoginResponse loginResponse = protocolPacket.responselogin();
            return this->handleLoginResponse(packet, &protocolPacket, &loginResponse);
        }

        // user information packet
        case Protocol::Packet_PacketType_UserInformations :
        {
            emit this->userInformationsReceived(protocolPacket.userinformations());
        }

        default :
        {
            break;
        }
    }

    // after handling packet delete it
    delete packet;
}

void SQMPacketProcessor::handleLoginResponse(DataPacket *dataPacket, Protocol::Packet *protocolPacket, Protocol::LoginResponse *loginResponse)
{
    // emit login response signal
    emit this->loginResponse(loginResponse->type() == Protocol::LoginResponse_Type_Success);
}
