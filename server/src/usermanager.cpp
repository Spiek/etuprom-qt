/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "usermanager.h"

//
// Con and decon
//
Usermanager::Usermanager(EleaphProtoRPC *eleaphRPC, QObject *parent) : QObject(parent)
{
    // save eleaphrpc
    this->eleaphRPC = eleaphRPC;

    // set default settings
    this->boolSettingsMultiSessionsActive = true;

    // protocol handlers
    this->connect(eleaphRPC, SIGNAL(sigDeviceRemoved(QIODevice*)), this, SLOT(handleClientDisconnect(QIODevice*)));
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_USER_LOGIN, this, SLOT(handleLogin(EleaphRpcPacket)));
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_USER_LOGOUT, this, SLOT(handleLogout(EleaphRpcPacket)), false, EleaphProcessEvent_Before(this));
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_USER_SELF_GET_INFO, this, SLOT(handleUserInfoSelf(EleaphRpcPacket)), false, EleaphProcessEvent_Before(this));

    // signal connections (this)
    this->connect(this, SIGNAL(sigUserChanged(Usermanager::SharedSession,QIODevice*,Usermanager::UserChangeType)), this, SLOT(handleUserChange(Usermanager::SharedSession,QIODevice*,Usermanager::UserChangeType)));
}

Usermanager::~Usermanager()
{
    // here we delete all sessions objects
    qDeleteAll(this->mapSocketsSession.values());

    // here we only delete the qmap "case"
    qDeleteAll(this->mapUsersSessions.values());
}


//
// External user managment helper methods
//

void Usermanager::addUserSession(QIODevice *device, Protocol::Session* session)
{
    // exit if object is invalid or session is allready logged in
    if(!session || this->mapSocketsSession.contains(device)) {
        return;
    }

    // save Usersession (Userid --> MAP(Socket, Session))
    Protocol::User *user = session->mutable_user();
    Usermanager::UserChangeType userChangeType = Usermanager::UserChangeType::UserSessionAdded;
    if(!this->mapUsersSessions.contains(user->id())) {
        userChangeType = (Usermanager::UserChangeType)((quint8)userChangeType | (quint8)Usermanager::UserChangeType::UserAdded);
        this->mapUsersSessions.insert(user->id(), new QMap<QIODevice*, Protocol::Session*>());
    }

    // ...and save session for later use
    this->mapUsersSessions.value(user->id())->insert(device, session);
    this->mapSocketsSession.insert(device, session);

    // Inform the outside world
    // will use a QSharedPointer version of a copy of the session object
    // so that we have a automatic garbage collection
    // and the protection of internal Usermanager objects
    Protocol::Session *newSession = new Protocol::Session(*session);
    emit this->sigUserChanged(Usermanager::SharedSession(newSession), device, userChangeType);
}

void Usermanager::removeUserSession(QIODevice *device)
{
    // exit if the device is invalid or if no session for device was found
    if(!device || !this->mapSocketsSession.contains(device)) {
        return;
    }

    // take session and user
    Protocol::Session *session = this->mapSocketsSession.take(device);
    Protocol::User *user = session->mutable_user();

    // remove One session from mapUserSessions
    QMap<QIODevice*, Protocol::Session*>* mapUserSession = this->mapUsersSessions.value(user->id());
    mapUserSession->remove(device);

    // if the last session was taken, so kill the session and whole user-session-map
    Usermanager::UserChangeType userChangeType = Usermanager::UserChangeType::UserSessionRemoved;
    if(mapUserSession->isEmpty()) {
        // clean whole mapUsersSessions map for userid
        this->mapUsersSessions.take(user->id());

        // add UserRemoved to change type
        userChangeType = (Usermanager::UserChangeType)((quint8)userChangeType | (quint8)Usermanager::UserChangeType::UserRemoved);

        // remove the user-session-map
        delete mapUserSession;
    }

    // Inform the outside world
    // will use a QSharedPointer version of the session object
    // so that we have an automatic garbage collection after all connected slots are called
    emit this->sigUserChanged(Usermanager::SharedSession(session), device, userChangeType);
}

bool Usermanager::isLoggedIn(QIODevice *device)
{
    return (device ? this->mapSocketsSession.contains(device) : false);
}

bool Usermanager::isLoggedIn(qint32 userID)
{
    return this->mapUsersSessions.contains(userID);
}


Protocol::Session* Usermanager::getConnectedSession(QIODevice *device)
{
    return this->mapSocketsSession.value(device, (Protocol::Session*)0);
}

QList<Protocol::Session*> Usermanager::getConnectedSessions(qint32 userid)
{
    QMap<QIODevice*, Protocol::Session*>* mapUserSessions = mapUsersSessions.value(userid, (QMap<QIODevice*, Protocol::Session*>*)0);
    return !mapUserSessions ? QList<Protocol::Session*>() : mapUserSessions->values();
}

QList<QIODevice*> Usermanager::getConnectedSessionSockets(qint32 userid)
{
    QMap<QIODevice*, Protocol::Session*>* mapUserSessions = mapUsersSessions.value(userid, (QMap<QIODevice*, Protocol::Session*>*)0);
    return !mapUserSessions ? QList<QIODevice*>() : mapUserSessions->keys();
}

Protocol::User* Usermanager::getConnectedUser(QIODevice *device)
{
    Protocol::Session *session = this->getConnectedSession(device);
    return session ? session->mutable_user() : (Protocol::User*)0;
}


//
// Settings setter/getter
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
// Post packet handling
//

void Usermanager::beforePacketProcessed(EleaphProtoRPC::Delegate *delegate, EleaphRpcPacket packet, bool* continueProcess)
{
    // do a login check
    Q_UNUSED(delegate);
    *continueProcess = this->isLoggedIn(packet.data()->ioPacketDevice);
}


//
// Packet Event handlers
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

    // construct Session (which will contain the matched user, and the session name)
    Protocol::Session *session = new Protocol::Session;
    Protocol::User *user = session->mutable_user();
    session->set_sessionname(request.sessionname());

    // if login data are not correct, so inform client by setting type to LoginResponse_Type_LoginIncorect
    Protocol::LoginResponse response;
    if(!Global::getDatabaseHelper()->getUserByIdUserNameAndPw(strUsername, strPassword, user)) {
        response.set_type(Protocol::LoginResponse_Type_LoginIncorect);
    }

    // otherwise if "multi Sessions are not allowed" AND "user is allready logged in", so inform client by setting type to LoginResponse_Type_AllreadyLoggedIn
    else if(!this->boolSettingsMultiSessionsActive && this->isLoggedIn(user->id())) {
        response.set_type(Protocol::LoginResponse_Type_AllreadyLoggedIn);

    }

    // otherwise we have a successfull login so we inform client by setting type to LoginResponse_Type_Success
    else {
        response.set_type(Protocol::LoginResponse_Type_Success);
    }

    // of course if login wasn't successfull, delete the constructed session
    if(response.type() != Protocol::LoginResponse_Type_Success) {
        delete session;
        return;
    }

    // otherwise log user in
    else {
        this->addUserSession(dataPacket.data()->ioPacketDevice, session);
    }

    // send login response to client
    this->eleaphRPC->sendRPCDataPacket(dataPacket.data()->ioPacketDevice, PACKET_DESCRIPTOR_USER_LOGIN, response.SerializeAsString());
}

void Usermanager::handleLogout(EleaphRpcPacket dataPacket)
{
    this->removeUserSession(dataPacket.data()->ioPacketDevice);
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


//
// Low Level Event handlers
//

void Usermanager::handleClientDisconnect(QIODevice *device)
{
    this->removeUserSession(device);
}

void Usermanager::handleUserChange(Usermanager::SharedSession session, QIODevice *deviceProducerOfChange, Usermanager::UserChangeType changeType)
{
    // unused....
    Q_UNUSED(deviceProducerOfChange)

    // just update the "online" state of user, if user has his first login or last logout
    Protocol::User* user = session.data()->mutable_user();
    if(((quint8)changeType & (quint8)Usermanager::UserChangeType::UserAdded) ||
       ((quint8)changeType & (quint8)Usermanager::UserChangeType::UserRemoved))
    {
        user->set_online(((quint8)changeType & (quint8)Usermanager::UserChangeType::UserAdded));
        Global::getDatabaseHelper()->updateUserOnlineState(user);
    }
}
