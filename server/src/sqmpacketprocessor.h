#ifndef SQMPACKETPROCESSOR_H
#define SQMPACKETPROCESSOR_H

// forward declaration
class SQMPacketProcessor;

// Qt (core)
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QIODevice>
#include <QtCore/QPair>

// own libs
#include "SQMPacketHandler"
#include "usermanager.h"

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
};

#endif // SQMPACKETPROCESSOR_H
