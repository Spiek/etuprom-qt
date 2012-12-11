/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#ifndef GLOBAL_H
#define GLOBAL_H

// Qt (core)
#include <QtCore/QCoreApplication>

// own libs
#include "packetprocessor.h"
#include "databasehelper.h"
#include "EleaphProtoRpc"

class Global
{
    public:
        // global initializer
        static void initialize();

        // global object getter
        static PacketProcessor* getPPInstance();
        static EleaphProtoRPC* getERPCInstance();
        static DatabaseHelper* getDatabaseHelper();

    private:
        // settings
        static bool init;
        static quint16 intListenPort;

        // SINGELTON objects
        static PacketProcessor *packetProcessor;
        static EleaphProtoRPC *eleaphRPC;
        static DatabaseHelper* databaseHelper;
};

#endif // GLOBAL_H
