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
#include "usermanager.h"
#include "EleaphProtoRpc"
#include "global.h"

class SQMPacketProcessor : public QObject
{
    Q_OBJECT
    struct User
    {
        qint32 id;
        QString userName;
        qint32 state;
        bool online;
        bool visible;
    };

    public:
        // con/decon
        SQMPacketProcessor(QObject *parent = 0);
        ~SQMPacketProcessor();

    private slots:
        // protocol handler methods
        void handleLogin(DataPacket* dataPacket);
        void handleUserMessage(DataPacket* rpcPacket);
};

#endif // SQMPACKETPROCESSOR_H
