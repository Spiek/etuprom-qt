/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#include "usermanager.h"

Usermanager::Usermanager(PacketProcessor *packetProcessor, QObject *parent) : QObject(parent)
{
    // save eleaphrpc
    this->packetProcessor = packetProcessor;

    // handle client disconnects
    EleaphProtoRPC* eleaphRpc = packetProcessor->getEleaphRpc();
    this->connect(eleaphRpc, SIGNAL(sigDeviceRemoved(QIODevice*)), this, SLOT(handle_client_disconnect(QIODevice*)));
    eleaphRpc->registerRPCMethod(PACKET_DESCRIPTOR_USER_LOGIN, this, SLOT(handleLogin(EleaphRPCDataPacket*)));
    eleaphRpc->registerRPCMethod(PACKET_DESCRIPTOR_USER_LOGOUT, this, SLOT(handleLogout(EleaphRPCDataPacket*)));
    eleaphRpc->registerRPCMethod(PACKET_DESCRIPTOR_USER_SELF_GET_INFO, this, SLOT(handleUserInfoSelf(EleaphRPCDataPacket*)));
}

Usermanager::~Usermanager()
{

}




//
// Section:
//  Protocol helper methods
//

void Usermanager::userChanged(qint32 userid, bool refreshUser)
{
    return this->userChanged(this->getConnectedDevice(userid), refreshUser);
}

void Usermanager::userChanged(QIODevice *device, bool refreshUser)
{
    // if we have a user change on a non logged in user, exit
    Protocol::User* userChanged = this->getConnectedUser(device);
    if(!userChanged) {
        return;
    }

    // refresh user if wanted
    if(refreshUser) {
        this->refreshUser(userChanged);
    }

    // tell the outside world about user change
    emit this->sigUserChanged(userChanged, device, refreshUser);
}


//
// Section:
//  User managment helper methods
//

void Usermanager::addUser(QIODevice *device, Protocol::User *user)
{
    // exit if the user does exist or the user is invalid
    if(!user || this->mapIdUser.contains(user->id())) {
        return;
    }

    // add to socket -> user map
    this->mapSocketUser.insert(device, user);

    // add to userid -> QPair(device, user) map
    this->mapIdUser.insert(user->id(), QPair<QIODevice*, Protocol::User*>(device, user));

    // update the user in database
    user->set_online(true);
    Global::getDatabaseHelper()->updateUserOnlineStateById(user, true);

    // ... and inform all users which belongs to the user's contacht list about users state change
    this->userChanged(user->id(), false);
}

void Usermanager::removeUser(QIODevice *device)
{
    // take from socket -> user map
    Protocol::User *user = this->mapSocketUser.value(device);

    // call overloaded method
    return this->removeUser(user);
}

void Usermanager::removeUser(Protocol::User *user)
{
    // exit if the user doesn't exist or the user is invalid
    if(!user || !this->mapIdUser.contains(user->id())) {
        return;
    }

    // update the user in database
    user->set_online(false);
    Global::getDatabaseHelper()->updateUserOnlineStateById(user, false);

    // ... and inform all users which belongs to the user's contact list about users state change
    this->userChanged(user->id(), false);

    // REMOVE USER

    // remove from userid -> QPair(device, user) map
    QPair<QIODevice*, Protocol::User*> pairValueUser = this->mapIdUser.take(user->id());

    // remove from socket -> user map
    this->mapSocketUser.take(pairValueUser.first);

    // delete user
    delete pairValueUser.second;
}

bool Usermanager::isLoggedIn(QIODevice *device)
{
    return (device ? this->mapSocketUser.contains(device) : false);
}

bool Usermanager::isLoggedIn(qint32 userID)
{
    return this->mapIdUser.contains(userID);
}

bool Usermanager::refreshUser(Protocol::User *user)
{
    return Global::getDatabaseHelper()->getUserById(user->id(), user);
}

Protocol::User* Usermanager::getConnectedUser(QIODevice *device)
{
    return this->mapSocketUser.value(device, (Protocol::User*)0);
}

Protocol::User* Usermanager::getConnectedUser(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>((QIODevice*)0, (Protocol::User*)0));
    return pairSocketUser.second;
}

QIODevice* Usermanager::getConnectedDevice(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>((QIODevice*)0, (Protocol::User*)0));
    return pairSocketUser.first;
}

//
// Signal handlings
//

void Usermanager::handle_client_disconnect(QIODevice *device)
{
    // if the given device is logged in, log the device/user off
    if(this->mapSocketUser.contains(device)) {
        this->removeUser(device);
    }
}

//
// Packet handlings
//

void Usermanager::handleLogin(EleaphRpcPacket dataPacket)
{
    // simplefy login packet values
    Protocol::LoginRequest request;
    if(!request.ParseFromArray(dataPacket.data->baRawPacketData->constData(),  dataPacket.data->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse LoginRequest", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }
    QString strUsername = QString::fromStdString(request.username());
    QString strPassword  = QString::fromStdString(request.password());

    // construct Default User (which will contain the matched user, if found)
    Protocol::User *user = new Protocol::User;

    // simplefy object access
    DatabaseHelper *databaseHelper = Global::getDatabaseHelper();

    // inform the client if the user was found, or not
    Protocol::LoginResponse response;
    if(!databaseHelper->getUserByIdUserNameAndPw(strUsername, strPassword, user)) {
        response.set_type(Protocol::LoginResponse_Type_LoginIncorect);
    } else {
        response.set_type(Protocol::LoginResponse_Type_Success);
    }
    this->packetProcessor->getEleaphRpc()->sendRPCDataPacket(dataPacket.data->ioPacketDevice, PACKET_DESCRIPTOR_USER_LOGIN, response.SerializeAsString());

    // if login wasn't successfull, delte constructed user and end here
    if(response.type() == Protocol::LoginResponse_Type_LoginIncorect) {
        delete user;
        return;
    }

    // add user to the usermanager
    this->addUser(dataPacket.data->ioPacketDevice, user);
}

void Usermanager::handleUserInfoSelf(EleaphRpcPacket dataPacket)
{
    // get the logged in user, if the given user is not logged in, don't handle the packet
    Protocol::User *user = this->mapSocketUser.value(dataPacket.data->ioPacketDevice, (Protocol::User*)0);
    if(!user) {
        return;
    }

    // send the user it's user data
    this->packetProcessor->getEleaphRpc()->sendRPCDataPacket(dataPacket.data->ioPacketDevice, PACKET_DESCRIPTOR_USER_SELF_GET_INFO, user->SerializeAsString());
}

void Usermanager::handleLogout(EleaphRpcPacket dataPacket)
{
    // if the given device is logged in, log the device/user off
    QIODevice* device = dataPacket.data->ioPacketDevice;
    if(this->mapSocketUser.contains(device)) {
        this->removeUser(device);
    }
}
