#include "sqmpacketprocessor.h"

SQMPacketProcessor::SQMPacketProcessor(QObject *parent) : QObject(parent)
{
    this->connect(Global::packetHandler, SIGNAL(deviceUsageChanged(QIODevice*,bool)), this, SLOT(clientStreamChanged(QIODevice*,bool)));
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
        {
            Protocol::LoginRequest loginRequest = protocolPacket.requestlogin();
            return this->handleLogin(packet, &protocolPacket, &loginRequest);
            break;
        }
        default :
        {
            break;
        }
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
    QSqlQuery query = this->dbLogin(username, password);

    // create default response packet
    Protocol::Packet packetResponse;
    packetResponse.set_packettype(Protocol::Packet_PacketType_LoginResponse);
    Protocol::LoginResponse *loginResponse = packetResponse.mutable_responselogin();

    // login was success
    Protocol::User *user = 0;
    if(query.next()) {
        // create new user struct
        user = new Protocol::User;
        user->set_id(query.value(0).toInt());
        user->set_username(query.value(1).toString().toStdString());
        user->set_state(query.value(2).toInt());
        user->set_online(query.value(3).toBool());
        user->set_visible(query.value(4).toBool());
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

    // exit here if login was not correct
    if(!user) {
        return;
    }

    // update user state in database
    this->dbUpdateUserOnOfflineState(user->id(), true);

    //
    // send UserInformations
    //

    // build UserInformations protocol packet
    Protocol::Packet packetUserInformations;
    packetUserInformations.set_packettype(Protocol::Packet_PacketType_UserInformations);
    Protocol::UserInformations *userInformation = packetUserInformations.mutable_userinformations();

    // set user informations
    userInformation->mutable_user()->MergeFrom(*user);

    // select all contacts which are in the contactlist of connection user
    query = this->dbGetContactList(user->id());

    // ... and add them to the protobuf contactlist
    while(query.next()) {
        Protocol::Contact *contact = userInformation->add_contact();
            Protocol::User *user = contact->mutable_user();
            user->set_id(query.value(0).toInt());
            user->set_username(query.value(1).toString().toStdString());
            user->set_state(query.value(2).toInt());
            user->set_online(query.value(3).toBool());
            user->set_visible(query.value(4).toBool());
        contact->set_group(query.value(5).toString().toStdString());
    }

    // send userinformation packet
    Global::packetHandler->sendDataPacket(dataPacket->ioPacketDevice, packetUserInformations.SerializeAsString());
}

void SQMPacketProcessor::clientStreamChanged(QIODevice *device, bool used)
{
    // if we have a disconnect and user was logged in,
    // remove user from cached user list and update on/offline state in db
    if(!used &&  this->mapUserData.contains(device)) {
        Protocol::User *user = this->mapUserData.take(device);
        this->dbUpdateUserOnOfflineState(user->id(), false);
        delete user;
    }
}


//
// Database helper method section
//
QSqlQuery SQMPacketProcessor::dbLogin(QString strUserName, QString strPassword)
{
    // build query
    QString strQuery = QString(
                "SELECT "
                "	id, "
                "	username, "
                "	state, "
                "	online, "
                "	visible "
                "FROM users "
                "WHERE "
                "	username = \"%1\" AND "
                "	password = \"%2\"; "
    ).arg(strUserName, strPassword);

    // exec query and return the result
    return QSqlQuery(strQuery, QSqlDatabase::database());
}

bool SQMPacketProcessor::dbUpdateUserOnOfflineState(qint32 intId, bool online)
{
    // build query
    QString strQuery = QString(
                "UPDATE "
                "	users SET online = %1 "
                "WHERE id = %2 "
    ).arg((int)online).arg(intId);

    // exec query and return the result
    return QSqlQuery(strQuery, QSqlDatabase::database()).exec();
}

QSqlQuery SQMPacketProcessor::dbGetContactList(qint32 intId)
{
    // build query
    QString strQuery = QString(
                "SELECT "
                "	users.id, "
                "	users.username, "
                "	users.state, "
                "	users.online, "
                "	users.visible, "
                "	userlist.group "
                "FROM users "
                "INNER JOIN userlist "
                "ON 	userlist.user_id = %1 AND "
                "		users.id = userlist.contact_user_id; "
    ).arg(intId);

    // exec query and return the result
    return QSqlQuery(strQuery, QSqlDatabase::database());
}
