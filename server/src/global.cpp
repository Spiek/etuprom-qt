#include "global.h"

// init static vars
SQMPacketHandler* Global::packetHandler = 0;
SQMPacketProcessor* Global::packetProcessor = 0;
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
    if(DatabaseHelper::initialize("QODBC", "sqm")) {
        printf("Datenbank Verbindung erfolgreich!");
    } else {
        qFatal("Datenbank Verbindung NICHT erfolgreich!");
    }

    // initialize packet handler, which handled the packet parsing
    SQMPacketHandler::create(app);
    Global::packetHandler = SQMPacketHandler::getInstance();

    // and start tcp listening
    Global::packetHandler->startTcpListening(Global::intListenPort);

    // initialize packet processor, which process the packets
    Global::packetProcessor = new SQMPacketProcessor(app);
    app->connect(Global::packetHandler, SIGNAL(newPacketReceived(DataPacket*)), Global::packetProcessor, SLOT(newPacketReceived(DataPacket*)));
    app->connect(Global::packetHandler, SIGNAL(deviceUsageChanged(QIODevice*,bool)), Global::packetProcessor, SLOT(clientUsageChanged(QIODevice*,bool)));

    // class was successfull initialized!
    Global::init = true;
}
