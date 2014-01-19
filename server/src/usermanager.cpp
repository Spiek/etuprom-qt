/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "usermanager.h"

Usermanager::Usermanager(EleaphProtoRPC *eleaphRPC, QObject *parent) : QObject(parent)
{
    // save eleaphrpc
    this->eleaphRPC = eleaphRPC;

    // set default settings
    this->boolSettingsMultiSessionsActive = true;

    // handle client disconnects
    this->connect(eleaphRPC, SIGNAL(sigDeviceRemoved(QIODevice*)), this, SLOT(handle_client_disconnect(QIODevice*)));
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_USER_LOGIN, this, SLOT(handleLogin(EleaphRPCDataPacket*)));
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_USER_LOGOUT, this, SLOT(handleLogout(EleaphRPCDataPacket*)));
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_USER_SELF_GET_INFO, this, SLOT(handleUserInfoSelf(EleaphRPCDataPacket*)));

    // signal connections (this)
    this->connect(this, SIGNAL(sigUserChanged(Usermanager::UserShared,QIODevice*,Usermanager::UserChangeType)), this, SLOT(handleUserChange(Usermanager::UserShared,QIODevice*,Usermanager::UserChangeType)));
}

Usermanager::~Usermanager()
{
    // here we delete all user objects
    qDeleteAll(this->mapSocketsUser.values());

    // here we only delete the qmap "case"
    qDeleteAll(this->mapUsersSessions.values());
}

//
//  User managment helper methods
//

void Usermanager::addUserSession(QIODevice *device, Protocol::User* user)
{
    // exit if the user is invalid or if user is allready logged in with device
    if(!user || this->mapSocketsUser.contains(device)) {
        return;
    }

    // save Usersession (Userid --> MAP(Socket, User))
    Usermanager::UserChangeType userChangeType = Usermanager::UserChangeType::UserSessionAdded;
    if(!this->mapUsersSessions.contains(user->id())) {
        userChangeType = (Usermanager::UserChangeType)((quint8)userChangeType | (quint8)Usermanager::UserChangeType::UserAdded);
        this->mapUsersSessions.insert(user->id(), new QMap<QIODevice*, Protocol::User*>());
    }

    // otherwise create a new Session (the user is allready logged in so delte the new user object and take the existing one)
    else {
        Protocol::User* userDelete = user;
        user = this->mapUsersSessions.value(user->id())->first();
        delete userDelete;
    }
    this->mapUsersSessions.value(user->id())->insert(device, user);

    // save Socketsession (Socket --> User)
    this->mapSocketsUser.insert(device, user);

    // ... and inform the outside world (will use a QSharedPointer version of a copy of the user object, so that we have a automatic garbage collection!)
    Protocol::User *newUser = new Protocol::User(*user);
    emit this->sigUserChanged(Usermanager::UserShared(newUser), device, userChangeType);
}

void Usermanager::removeUserSession(QIODevice *device)
{
    // exit if the device is invalid or if no session for device was found
    if(!device || !this->mapSocketsUser.contains(device)) {
        return;
    }

    // take session from mapSocketUser
    Protocol::User *user = this->mapSocketsUser.take(device);

    // remove session from mapUserSessions
    QMap<QIODevice*, Protocol::User*>* mapUserSession = this->mapUsersSessions.value(user->id());
    mapUserSession->take(device);

    // if the last session was taken, so kill the whole user (including usersessionmap instance)
    Usermanager::UserChangeType userChangeType = Usermanager::UserChangeType::UserSessionRemoved;
    if(mapUserSession->isEmpty()) {
        // remove the user id from mapUsersSessions
        this->mapUsersSessions.take(user->id());

        // generate correct userChangeType
        userChangeType = (Usermanager::UserChangeType)((quint8)userChangeType | (quint8)Usermanager::UserChangeType::UserRemoved);

        // remove the mapuserSession "case"
        delete mapUserSession;
    }

    // inform the outside world (will use a QSharedPointer version of a copy of the user object, so that we have a automatic garbage collection!)
    this->sigUserChanged(Usermanager::UserShared(user), device, userChangeType);
}

bool Usermanager::isLoggedIn(QIODevice *device)
{
    return (device ? this->mapSocketsUser.contains(device) : false);
}

bool Usermanager::isLoggedIn(qint32 userID)
{
    return this->mapUsersSessions.contains(userID);
}

Protocol::User* Usermanager::getConnectedUser(QIODevice *device)
{
    return this->mapSocketsUser.value(device, (Protocol::User*)0);
}

Protocol::User* Usermanager::getConnectedUser(qint32 userid)
{
    QMap<QIODevice*, Protocol::User*>* mapUserSessions = mapUsersSessions.value(userid, (QMap<QIODevice*, Protocol::User*>*)0);
    return !mapUserSessions ? (Protocol::User*)0 : mapUserSessions->first();
}

QIODevice* Usermanager::getConnectedDevice(qint32 userid)
{
    QMap<QIODevice*, Protocol::User*>* mapUserSessions = mapUsersSessions.value(userid, (QMap<QIODevice*, Protocol::User*>*)0);
    return !mapUserSessions ? (QIODevice*)0 : mapUserSessions->firstKey();
}

//
// Signal handlings
//

void Usermanager::handleUserChange(Usermanager::UserShared userChanged, QIODevice *deviceProducerOfChange, Usermanager::UserChangeType changeType)
{
    // unused....
    Q_UNUSED(deviceProducerOfChange)

    // just update the "online" state of user, if user has his first login or last logout
    Protocol::User* user = userChanged.data();
    if(((quint8)changeType & (quint8)Usermanager::UserChangeType::UserAdded) ||
       ((quint8)changeType & (quint8)Usermanager::UserChangeType::UserRemoved))
    {
        userChanged.data()->set_online(((quint8)changeType & (quint8)Usermanager::UserChangeType::UserAdded));
        Global::getDatabaseHelper()->updateUserOnlineState(user);
    }
}

void Usermanager::handle_client_disconnect(QIODevice *device)
{
    this->removeUserSession(device);
}

//
// Settings (setter/getter)
//

void Usermanager::setSettingsActivateMultiSessions(bool enabled)
{
    this->boolSettingsMultiSessionsActive = enabled;
}

bool Usermanager::getSettingsActivateMultiSession()
{
    return this->boolSettingsMultiSessionsActive;
}


//
// Packet handlings
//
void Usermanager::handleLogin(EleaphRpcPacket dataPacket)
{
    // simplefy login packet values
    Protocol::LoginRequest request;
    if(!request.ParseFromArray(dataPacket.data()->baRawPacketData->constData(),  dataPacket.data()->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse LoginRequest", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }
    QString strUsername = QString::fromStdString(request.username());
    QString strPassword  = QString::fromStdString(request.password());

    // construct Default User (which will contain the matched user, if found)
    Protocol::User *user = new Protocol::User;

    // inform the client if the user was found, or not
    Protocol::LoginResponse response;
    if(!Global::getDatabaseHelper()->getUserByIdUserNameAndPw(strUsername, strPassword, user)) {
        response.set_type(Protocol::LoginResponse_Type_LoginIncorect);
    } else if(this->boolSettingsMultiSessionsActive || !this->isLoggedIn(user->id())) {
        response.set_type(Protocol::LoginResponse_Type_Success);
    }  else {
        response.set_type(Protocol::LoginResponse_Type_AllreadyLoggedIn);
    }
    this->eleaphRPC->sendRPCDataPacket(dataPacket.data()->ioPacketDevice, PACKET_DESCRIPTOR_USER_LOGIN, response.SerializeAsString());

    // if login wasn't successfull, delte constructed user and end here
    if(response.type() != Protocol::LoginResponse_Type_Success) {
        delete user;
        return;
    }

    // add user to the usermanager
    this->addUserSession(dataPacket.data()->ioPacketDevice, user);
}

void Usermanager::handleUserInfoSelf(EleaphRpcPacket dataPacket)
{
    // get the logged in user, if the given user is not logged in, don't handle the packet
    Protocol::User *user = this->getConnectedUser(dataPacket.data()->ioPacketDevice);
    if(!user) {
        return;
    }

    // send the user it's user data
    this->eleaphRPC->sendRPCDataPacket(dataPacket.data()->ioPacketDevice, PACKET_DESCRIPTOR_USER_SELF_GET_INFO, user->SerializeAsString());
}

void Usermanager::handleLogout(EleaphRpcPacket dataPacket)
{
    this->removeUserSession(dataPacket.data()->ioPacketDevice);
}
