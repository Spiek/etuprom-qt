#ifndef GLOBAL_H
#define GLOBAL_H

// Qt (core)
#include <QtCore/QCoreApplication>

// own libs
#include "sqmpacketprocessor.h"
#include "databasehelper.h"
#include "usermanager.h"
#include "EleaphProtoRpc"

// forward delclarion, becuase of cyrcle including of the usermanager!
class Usermanager;

class Global
{
    public:
        // global initializer
        static void initialize();

        // global object getter
        static SQMPacketProcessor* getPPInstance();
        static EleaphProtoRPC* getERPCInstance();
        static Usermanager* getUserManager();
        static DatabaseHelper* getDatabaseHelper();

    private:
        // settings
        static bool init;
        static quint16 intListenPort;

        // SINGELTON objects
        static SQMPacketProcessor *packetProcessor;
        static Usermanager *userManager;
        static EleaphProtoRPC *eleaphRPC;
        static DatabaseHelper* databaseHelper;
};

#endif // GLOBAL_H
