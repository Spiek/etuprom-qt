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

    // handle packets which will send before user is logged in
    if(!this->loggedIn) {
        // user login result
        if(packetType == Protocol::Packet_PacketType_LoginResponse) {
            Protocol::LoginResponse loginResponse = protocolPacket.responselogin();
            return this->handleLoginResponse(packet, &protocolPacket, &loginResponse);
        }
    }

    // handle packets which will send after user is successfull logged in
    else {
        // user information packet
        if(packetType == Protocol::Packet_PacketType_UserInformations) {
            emit this->userInformationsReceived(protocolPacket.userinformations());
        }

        // user altered packet
        else if(packetType == Protocol::Packet_PacketType_UserAltered) {
            emit this->userAltered(protocolPacket.useraltered());
        }
    }

    // after handling packet delete it
    delete packet;
}

void SQMPacketProcessor::handleLoginResponse(DataPacket *dataPacket, Protocol::Packet *protocolPacket, Protocol::LoginResponse *loginResponse)
{
    this->loggedIn = (loginResponse->type() == Protocol::LoginResponse_Type_Success);

    // emit login response signal
    emit this->loginResponse(this->loggedIn);
}
