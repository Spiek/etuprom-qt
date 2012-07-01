#include "global.h"

// init static vars
SQMPeerHandler* Global::peerHandler = 0;
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

    // init database connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "default");
    db.setDatabaseName("sqm");
    if(db.open()) {
        printf("Datenbank Verbindung erfolgreich!");
    } else {
        qFatal("Datenbank Verbindung NICHT erfolgreich!");
    }

    // initialize peer handler, which handled the peers
    Global::peerHandler = new SQMPeerHandler(Global::intListenPort, QHostAddress::Any, app);
    Global::peerHandler->listen();

    // initialize packet handler, which handled the packet parsing
    SQMPacketHandler::create(app);
    Global::packetHandler = SQMPacketHandler::getInstance();
    app->connect(Global::peerHandler, SIGNAL(newDevice(QIODevice*)), Global::packetHandler, SLOT(addDevice(QIODevice*)));

    // initialize packet processor, which process the packets
    Global::packetProcessor = new SQMPacketProcessor(app);
    app->connect(Global::packetHandler, SIGNAL(newPacketReceived(DataPacket*)), Global::packetProcessor, SLOT(newPacketReceived(DataPacket*)));

    // class was successfull initialized!
    Global::init = true;
}
