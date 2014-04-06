#ifndef LOGINPROTOCOLCONTROLLER_H
#define LOGINPROTOCOLCONTROLLER_H

// Qt
#include <QtCore/QObject>
#include <QtCore/QCryptographicHash>

// own (globals)
#include "global.h"

// own (protbuf)
#include "protocol.pb.h"
#include "collective/proto/packettypes.h"

class LoginProtocolController : public QObject
{
    Q_OBJECT
    public:
        enum class ConnectionState
        {
            Connecting,
            Connected,
            ConnectionError,
            Disconnected
        };
        LoginProtocolController(QObject *parent = 0);

    signals:
        void connectionChanged(QTcpSocket *socketServer, LoginProtocolController::ConnectionState connectionState);

    public slots:
        void connectToServer();
        Protocol::LoginResponse login(QString strUsername, QString strPassword, QString strSession);
        Protocol::User* fetchOwnUserData();
        void fetchContactList();

    private slots:
        void socketConnectionStateChanged(QAbstractSocket::SocketState stateSocket);
        void socketConnectionError();
        void socketConnectionDisconnected();
};

#endif // LOGINPROTOCOLCONTROLLER_H
