#ifndef SQMPACKETPROCESSOR_H
#define SQMPACKETPROCESSOR_H

// forward declaration
class SQMPacketProcessor;

// Qt (core)
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QIODevice>

// Qt (sql)
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

// own libs
#include "global.h"

// protobuf libs
#include "protocol.pb.h"


struct User
{
    quint32 intId;
    QString strUsername;
    QIODevice *ioPeer;
};

class SQMPacketProcessor : public QObject
{
    Q_OBJECT
    public:
        SQMPacketProcessor(QObject *parent = 0);

    private:
        QMap<QIODevice*, User*> mapUserData;

    public slots:
        void newPacketReceived(DataPacket *packet);

        // handler methods
        void handleLogin(DataPacket *dataPacket, Protocol::Packet *protocolPacket, Protocol::LoginRequest *login);
};

#endif // SQMPACKETPROCESSOR_H
