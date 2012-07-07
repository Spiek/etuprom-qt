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

class SQMPacketProcessor : public QObject
{
    Q_OBJECT
    public:
        SQMPacketProcessor(QObject *parent = 0);

    private:
        QMap<QIODevice*, Protocol::User*> mapUserData;

    public slots:
        void newPacketReceived(DataPacket *packet);
        void clientStreamChanged(QIODevice* device, bool used);

        // handler helper methods
        void handleLogin(DataPacket *dataPacket, Protocol::Packet *protocolPacket, Protocol::LoginRequest *login);

        // database helper methods
        QSqlQuery dbLogin(QString strUserName, QString strPassword);
        bool dbUpdateUserOnOfflineState(qint32 intId, bool online);
        QSqlQuery dbGetContactList(qint32 intId);

};

#endif // SQMPACKETPROCESSOR_H
