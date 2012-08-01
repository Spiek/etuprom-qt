#ifndef GLOBAL_H
#define GLOBAL_H

// Qt (core)
#include <QtCore/QCoreApplication>

// own libs
#include "SQMPacketHandler"
#include "sqmpacketprocessor.h"
#include "databasehelper.h"
#include "usermanager.h"

class Global
{
    public:
        static SQMPacketProcessor *packetProcessor;

        static quint16 intListenPort;
        static void initialize();

    private:
        static bool init;
};

#endif // GLOBAL_H
