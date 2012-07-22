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
    if(!used && this->getConnectedUser(device)) {
        this->removeUser(device);
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
        user = this->setUserfromQuery(&query);
        this->addUser(dataPacket->ioPacketDevice, user);

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
        this->setUserfromQuery(&query, contact->mutable_user());
        contact->set_group(query.value(5).toString().toStdString());
    }

    // send userinformation packet
    Global::packetHandler->sendDataPacket(dataPacket->ioPacketDevice, packetUserInformations.SerializeAsString());
}


//
// Section:
//  Protocol helper methods
//

void SQMPacketProcessor::userChanged(qint32 userid)
{
    return this->userChanged(this->getConnectedDevice(userid));
}

void SQMPacketProcessor::userChanged(QIODevice *device)
{
    // if we have a user change on a non logged in user, exit
    Protocol::User* userChanged = this->getConnectedUser(device);
    if(!userChanged) {
        return;
    }

    // refresh user
    this->refreshUser(userChanged);

    // build User Altered protocol packet and serialize it
    Protocol::Packet packetUserChange;
    packetUserChange.set_packettype(Protocol::Packet_PacketType_UserAltered);
    packetUserChange.mutable_useraltered()->MergeFrom(*userChanged);
    std::string strSerialzedPacket = packetUserChange.SerializeAsString();

    // select all users from changed user's contact list which are online and inform them about the change
    QSqlQuery query = DatabaseHelper::getOnlineContactsByUserId(userChanged->id());
    while(query.next()) {
        // get conntected user by id, and skip him if he isn't really contected
        Protocol::User *userOfContactList = this->getConnectedUser(query.value(0).toInt());
        if(!userOfContactList) {
            continue;
        }

        // inform user about the change
        QIODevice *device = this->getConnectedDevice(userOfContactList->id());

        // inform user about the change
        Global::packetHandler->sendDataPacket(device, strSerialzedPacket);
    }
}


//
// Section:
//  Database helper methods
//

Protocol::User* SQMPacketProcessor::setUserfromQuery(QSqlQuery *query,  Protocol::User* user)
{
    // construct target user if doesn't exist
    if(!user) {
        user = new Protocol::User;
    }

    // set the values from the query
    user->set_id(query->value(0).toInt());
    user->set_username(query->value(1).toString().toStdString());
    user->set_state(query->value(2).toInt());
    user->set_online(query->value(3).toBool());
    user->set_visible(query->value(4).toBool());

    // return new setted user
    return user;
}

//
// Section:
//  User managment helper methods
//

void SQMPacketProcessor::addUser(QIODevice *device, Protocol::User *user)
{
    // add to socket -> user map
    this->mapSocketUser.insert(device, user);

    // add to userid -> QPair(device, user) map
    this->mapIdUser.insert(user->id(), QPair<QIODevice*, Protocol::User*>(device, user));

    // update the user (in database and inform all users which belongs to the user about users state change)
    DatabaseHelper::updateUserOnlineState(user->id(), true);
    this->userChanged(user->id());
}

void SQMPacketProcessor::removeUser(QIODevice *device)
{
    // take from socket -> user map
    Protocol::User *user = this->mapSocketUser.value(device);

    // call overloaded method
    return this->removeUser(user);
}

void SQMPacketProcessor::removeUser(Protocol::User *user)
{
    // exit if the user doesn't exist or the user is invalid
    if(!user || !this->mapIdUser.contains(user->id())) {
        return;
    }

    // update the user (in database and inform all users which belongs to the user about users state change)
    DatabaseHelper::updateUserOnlineState(user->id(), false);
    this->userChanged(user->id());

    // remove from userid -> QPair(device, user) map
    QPair<QIODevice*, Protocol::User*> pairValueUser = this->mapIdUser.take(user->id());

    // remove from socket -> user map
    this->mapSocketUser.take(pairValueUser.first);

    // delete user
    delete pairValueUser.second;
}

Protocol::User* SQMPacketProcessor::refreshUser(Protocol::User *user)
{
    QSqlQuery queryUserRefresh = DatabaseHelper::getUserById(user->id());
    queryUserRefresh.next();
    return this->setUserfromQuery(&queryUserRefresh, user);
}

Protocol::User* SQMPacketProcessor::getConnectedUser(QIODevice *device)
{
    return this->mapSocketUser.value(device, 0);
}

Protocol::User* SQMPacketProcessor::getConnectedUser(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>(0, 0));
    return pairSocketUser.second;
}

QIODevice* SQMPacketProcessor::getConnectedDevice(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>(0, 0));
    return pairSocketUser.first;
}
