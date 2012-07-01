#ifndef GLOBAL_H
#define GLOBAL_H

// Qt (core)
#include <QtCore/QCoreApplication>

// own libs
#include "SQMPacketHandler"
#include "sqmpeerhandler.h"
#include "sqmpacketprocessor.h"

class Global
{
    public:
        static SQMPeerHandler *peerHandler;
        static SQMPacketHandler *packetHandler;
        static SQMPacketProcessor *packetProcessor;

        static quint16 intListenPort;
        static void initialize();

    private:
        static bool init;
};

#endif // GLOBAL_H
