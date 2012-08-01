#include "sqmpacketprocessor.h"


//
// Section:
//  Con's and Decon's
//

SQMPacketProcessor::SQMPacketProcessor(QObject *parent) : QObject(parent)
{

}

SQMPacketProcessor::~SQMPacketProcessor()
{

}


//
// Section:
//  public SLOTS (called by Qt's event system)
//

void SQMPacketProcessor::newPacketReceived(DataPacket *packet)
{
    // deserialize protobuf packet
    Protocol::Packet protocolPacket;
    protocolPacket.ParseFromArray(packet->baRawPacketData->constData(), packet->intPacktLength);
    Protocol::Packet_PacketType packetType = protocolPacket.packettype();

    // handle packet type
    if(packetType == Protocol::Packet_PacketType_LoginRequest) {
        this->handleLogin(packet, &protocolPacket);
    }

    // after handling packet delete it
    delete packet;
}

void SQMPacketProcessor::clientUsageChanged(QIODevice *device, bool used)
{
    // if we have a disconnect and user was logged in,
    // remove user from cached user list and update on/offline state in db
    if(!used && Usermanager::getInstance()->getConnectedUser(device)) {
        Usermanager::getInstance()->removeUser(device);
    }
}


//
// Section:
//  Protocol handler methods
//

void SQMPacketProcessor::handleLogin(DataPacket *dataPacket, Protocol::Packet *protocolPacket)
{
    // simplefy login packet values
    Protocol::LoginRequest* login = protocolPacket->mutable_requestlogin();
    QString username = QString::fromStdString(login->username()).toAscii();
    QString password  = QString::fromStdString(login->password()).toAscii();

    // search for user
    QSqlQuery query = DatabaseHelper::getUserByIdUserNameAndPw(username, password);

    // create response protobuf packet and fill it with default values
    Protocol::Packet packetResponse;
    packetResponse.set_packettype(Protocol::Packet_PacketType_LoginResponse);
    Protocol::LoginResponse *loginResponse = packetResponse.mutable_responselogin();

    // login was success
    Protocol::User *user = 0;
    if(query.next()) {
        // create new user
        user = Usermanager::getInstance()->setUserfromQuery(&query);
        Usermanager::getInstance()->addUser(dataPacket->ioPacketDevice, user);

        // login was succcessfull
        loginResponse->set_type(Protocol::LoginResponse_Type_Success);
    }

    // login was NOT success
    else {
        loginResponse->set_type(Protocol::LoginResponse_Type_LoginIncorect);
    }

    // send login response packet
    Global::packetHandler->sendDataPacket(dataPacket->ioPacketDevice, packetResponse.SerializeAsString());

    // exit here if login was not correct
    if(!user) {
        return;
    }

    //
    // send UserInformations
    //

    // build UserInformations protoful protocol packet and fill it with default values
    Protocol::Packet packetUserInformations;
    packetUserInformations.set_packettype(Protocol::Packet_PacketType_UserInformations);
    Protocol::UserInformations *userInformation = packetUserInformations.mutable_userinformations();

    // set user informations
    userInformation->mutable_user()->MergeFrom(*user);

    // select all contacts which are in the contactlist of connected user
    query = DatabaseHelper::getContactsByUserId(user->id());

    // ... and add them to the protobuf contactlist
    while(query.next()) {
        Protocol::Contact *contact = userInformation->add_contact();
        Usermanager::getInstance()->setUserfromQuery(&query, contact->mutable_user());
        contact->set_group(query.value(5).toString().toStdString());
    }

    // send userinformation packet
    Global::packetHandler->sendDataPacket(dataPacket->ioPacketDevice, packetUserInformations.SerializeAsString());
}
