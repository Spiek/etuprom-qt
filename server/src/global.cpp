/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#include "global.h"

// init static vars
SQMPacketProcessor* Global::packetProcessor = 0;
EleaphProtoRPC* Global::eleaphRPC = 0;
Usermanager* Global::userManager = 0;
DatabaseHelper* Global::databaseHelper = 0;

bool Global::init = false;

// Fixme: make it dynamic with a config file
quint16 Global::intListenPort = 1234;

void Global::initialize()
{
    // stop init process and exit if the class was allready initialized
    if(Global::init) {
        return;
    }

    // simplefy application instance
    QCoreApplication* app = QCoreApplication::instance();

    // init database helper
    QSqlDatabase database = QSqlDatabase::addDatabase("QODBC", "sqm");
    database.setDatabaseName("sqm");
    if(database.open()) {
        printf("Datenbank Verbindung erfolgreich!");
        Global::databaseHelper = new DatabaseHelper("sqm");
    } else {
        qFatal("Datenbank Verbindung NICHT erfolgreich!");
    }

    // initialize eleaph-proto-RPC-System
    Global::eleaphRPC = new EleaphProtoRPC(app, 65536);

    // initialize Usermanager
    Global::userManager = new Usermanager(app);

    // initialize packet processor, which process the packets
    Global::packetProcessor = new SQMPacketProcessor(app);

    // start the tcp listening
    Global::eleaphRPC->startTcpListening(Global::intListenPort);

    // class was successfull initialized!
    Global::init = true;
}

SQMPacketProcessor* Global::getPPInstance()
{
    return Global::packetProcessor;
}

EleaphProtoRPC* Global::getERPCInstance()
{
    return Global::eleaphRPC;
}

Usermanager* Global::getUserManager()
{
    return Global::userManager;
}

DatabaseHelper* Global::getDatabaseHelper()
{
    return Global::databaseHelper;
}
