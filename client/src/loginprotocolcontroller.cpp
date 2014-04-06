#include "loginprotocolcontroller.h"

LoginProtocolController::LoginProtocolController(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<LoginProtocolController::ConnectionState>("LoginProtocolController::ConnectionState");

    // handle server socket events
    this->connect(Global::socketServer, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketConnectionStateChanged(QAbstractSocket::SocketState)));
    this->connect(Global::socketServer, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketConnectionError()));
    this->connect(Global::socketServer, SIGNAL(disconnected()), this, SLOT(socketConnectionDisconnected()));
}

void LoginProtocolController::connectToServer()
{
    // simplefy data
    QTcpSocket *socketServer = Global::socketServer;

    // don't do anything, if we are allready connected
    if(socketServer->state() == QTcpSocket::ConnectedState) {
        return;
    }

    // (re)connect async to server
    socketServer->disconnectFromHost();
    socketServer->connectToHost(Global::strServerHostname, Global::intServerPort);
}

void LoginProtocolController::socketConnectionStateChanged(QAbstractSocket::SocketState stateSocket)
{
    // simplefy values
    QTcpSocket *socketServer = (QTcpSocket*)this->sender();

    // connecting
    if(  stateSocket == QAbstractSocket::HostLookupState ||
         stateSocket == QAbstractSocket::ConnectingState ||
         stateSocket == QAbstractSocket::BoundState)
    {
        emit this->connectionChanged(socketServer, ConnectionState::Connecting);
    }

    // connected
    else if( stateSocket == QAbstractSocket::ConnectedState) {
        emit this->connectionChanged(socketServer, ConnectionState::Connected);
    }
}

void LoginProtocolController::socketConnectionError()
{
    // simplefy values
    QTcpSocket *socketServer = (QTcpSocket*)this->sender();

    // error
    emit this->connectionChanged(socketServer, ConnectionState::ConnectionError);
}

void LoginProtocolController::socketConnectionDisconnected()
{
    // simplefy values
    QTcpSocket *socketServer = (QTcpSocket*)this->sender();

    // disconnect
    emit this->connectionChanged(socketServer, ConnectionState::Disconnected);
}

Protocol::LoginResponse LoginProtocolController::login(QString strUsername, QString strPassword, QString strSession)
{
    // create login protobuf objects
    Protocol::LoginRequest requestLogin;
    requestLogin.set_username(strUsername.toStdString());
    QByteArray baPasswordHash = QCryptographicHash::hash(strPassword.toUtf8(), QCryptographicHash::Sha1).toHex();
    requestLogin.set_password(baPasswordHash.data());
    requestLogin.set_sessionname(strSession.toStdString());

    // ... and send constructed protobuf packet to the server
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, PACKET_DESCRIPTOR_USER_LOGIN, requestLogin.SerializeAsString());

    // wait for packet async and process it
    EleaphRpcPacket epLoginResponse = Global::eleaphRpc->waitAsyncForPacket(PACKET_DESCRIPTOR_USER_LOGIN);

    // parse server response and handle it
    Protocol::LoginResponse responseLogin;
    responseLogin.ParseFromArray(epLoginResponse.data()->baRawPacketData->data(), epLoginResponse.data()->baRawPacketData->length());

    return responseLogin;
}

Protocol::User* LoginProtocolController::fetchOwnUserData()
{
    // aquire own user info's
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, PACKET_DESCRIPTOR_USER_SELF_GET_INFO);
    EleaphRpcPacket epUserSelf = Global::eleaphRpc->waitAsyncForPacket(PACKET_DESCRIPTOR_USER_SELF_GET_INFO);

    // parse and handle own user response
    Protocol::User *user = new Protocol::User;
    if(!user->ParseFromArray(epUserSelf.data()->baRawPacketData->constData(), epUserSelf.data()->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse User", __PRETTY_FUNCTION__ , __LINE__);
        delete user;
        return 0;
    }

    // return ready constructed user
    return user;
}

void LoginProtocolController::fetchContactList()
{
    // get contact list
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, PACKET_DESCRIPTOR_CONTACT_GET_LIST);
    EleaphRpcPacket epContactList = Global::eleaphRpc->waitAsyncForPacket(PACKET_DESCRIPTOR_CONTACT_GET_LIST);

    // if no contact list was given skip contact list handling
    Global::mapContactList.clear();
    if(epContactList.data()->baRawPacketData->length() > 0)  {
        // parse and handle contact list
        Protocol::ContactList contactList;
        if(!contactList.ParseFromArray(epContactList.data()->baRawPacketData->constData(), epContactList.data()->baRawPacketData->length())) {
            qWarning("[%s][%d] - Protocol Violation by Trying to Parse User", __PRETTY_FUNCTION__ , __LINE__);
            return;
        }

        // save all contacts for global access

        for(int i = 0; i < contactList.contact_size(); i++) {
            Protocol::Contact* contact = new Protocol::Contact(contactList.contact(i));
            Global::mapContactList.insert(contact->user().id(), contact);
            Global::mapCachedUsers.insert(contact->user().id(), contact->mutable_user());
        }
    }
}
