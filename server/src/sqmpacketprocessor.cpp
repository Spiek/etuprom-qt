#include "sqmpacketprocessor.h"

SQMPacketProcessor::SQMPacketProcessor(QObject *parent) : QObject(parent)
{

}

void SQMPacketProcessor::newPacketReceived(DataPacket *packet)
{
    // parse packet
    Protocol::Packet protocolPacket;
    protocolPacket.ParseFromArray(packet->baRawPacketData->constData(), packet->intPacktLength);
    Protocol::Packet_PacketType packetType = protocolPacket.packettype();

    // analyse type
    switch(packetType)
    {
        // login packet
        case Protocol::Packet_PacketType_LoginRequest :
            Protocol::LoginRequest loginRequest = protocolPacket.requestlogin();
            return this->handleLogin(packet, &protocolPacket, &loginRequest);
        break;
    }

    // after handling packet delete it
    delete packet;
}

void SQMPacketProcessor::handleLogin(DataPacket *dataPacket, Protocol::Packet *protocolPacket, Protocol::LoginRequest *login)
{
    // simplefy login packet values
    QString username = QString::fromStdString(login->username()).toAscii();
    QString password  = QString::fromStdString(login->password()).toAscii();

    // search for user
    QSqlDatabase database = QSqlDatabase::database("default");
    QString strQuery = QString("SELECT id FROM users WHERE username = \"%1\" AND password = \"%2\";").arg(username, password);
    qDebug() << strQuery;
    QSqlQuery query(strQuery, database);

    // create default response packet
    Protocol::Packet packetResponse;
    packetResponse.set_packettype(Protocol::Packet_PacketType_LoginResponse);
    Protocol::LoginResponse *loginResponse = packetResponse.mutable_responselogin();

    // login was success
    User* user = 0;
    if(query.next()) {
        // create new user struct
        user = new User;
        user->intId = query.value(0).toInt();
        user->strUsername = username;
        user->ioPeer = dataPacket->ioPacketDevice;
        this->mapUserData.insert(dataPacket->ioPacketDevice, user);

        // login is succcessfull
        loginResponse->set_type(Protocol::LoginResponse_Type_Success);
    }

    // login was NOT success
    else {
        loginResponse->set_type(Protocol::LoginResponse_Type_LoginIncorect);
    }

    // send login response packet
    Global::packetHandler->sendDataPacket(dataPacket->ioPacketDevice, packetResponse.SerializeAsString());

    // if login was correct, send userinformations
    if(user) {
        QString strQuery =
                " SELECT"
                "	users.id,"
                "	users.username,"
                "	users.state,"
                "	userlist.group"
                " FROM users"
                " INNER JOIN userlist"
                " ON userlist.user_id = %1 AND users.id = userlist.contact_user_id";
        strQuery = strQuery.arg(user->intId);
        QSqlQuery query(strQuery, database);

        // build the protocol packet
        Protocol::Packet packetUserInformations;
        packetUserInformations.set_packettype(Protocol::Packet_PacketType_UserInformations);
        Protocol::UserInformations *userInformation = packetUserInformations.mutable_userinformations();
        while(query.next()) {
            Protocol::Contact *contact = userInformation->add_contact();
            contact->set_id(query.value(0).toInt());
            contact->set_username(query.value(1).toString().toStdString());
            contact->set_state((Protocol::Contact::State)query.value(2).toInt());
            contact->set_group(query.value(3).toString().toStdString());
        }

        // send userinformation packet
        Global::packetHandler->sendDataPacket(dataPacket->ioPacketDevice, packetUserInformations.SerializeAsString());
    }
}
