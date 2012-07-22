#ifndef SQMPACKETPROCESSOR_H
#define SQMPACKETPROCESSOR_H

// forward declaration
class SQMPacketProcessor;

// Qt (core)
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QIODevice>
#include <QtCore/QPair>

// Qt (sql)
#include <QtSql/QSqlQuery>

// own libs
#include "global.h"

// protobuf libs
#include "protocol.pb.h"

class SQMPacketProcessor : public QObject
{
    Q_OBJECT
    public:
        // con/decon
        SQMPacketProcessor(QObject *parent = 0);
        ~SQMPacketProcessor();

    private:
        QMap<QIODevice*, Protocol::User*> mapSocketUser;
        QMap<qint32, QPair<QIODevice*, Protocol::User*> > mapIdUser;

    public slots:
        void newPacketReceived(DataPacket *packet);
        void clientUsageChanged(QIODevice* device, bool used);

    private:
        // protocol handler methods
        void handleLogin(DataPacket *dataPacket, Protocol::Packet *protocolPacket);

        // protocol helper methods
        void userChanged(QIODevice *device);
        void userChanged(qint32 userid);

        // database helper methods
        Protocol::User* setUserfromQuery(QSqlQuery *query,  Protocol::User* user = 0);

        // user managment helper methods
        void addUser(QIODevice *device, Protocol::User *user);
        void removeUser(Protocol::User *user);
        void removeUser(QIODevice *device);
        Protocol::User* refreshUser(Protocol::User *user);
        Protocol::User* getConnectedUser(QIODevice *device);
        Protocol::User* getConnectedUser(qint32 userid);
        QIODevice* getConnectedDevice(qint32 userid);
};

#endif // SQMPACKETPROCESSOR_H
