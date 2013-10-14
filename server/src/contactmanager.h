#ifndef CONTACTMANAGER_H
#define CONTACTMANAGER_H

#include <QObject>

// own libs
#include "packetprocessor.h"
#include "protocol.pb.h"
#include "global.h"

class Contactmanager : public QObject
{
    Q_OBJECT
    public:
        // con and decon
        Contactmanager(PacketProcessor *packetProcessor, QObject *parent = 0);

    private:
        PacketProcessor *packetProcessor;

    private slots:
        void handleContactList(EleaphRPCDataPacket* dataPacket);
        void handleContactChange(Protocol::User* userChanged, QIODevice *deviceProducerOfChange);
};

#endif // CONTACTMANAGER_H
