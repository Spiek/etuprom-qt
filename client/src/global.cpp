/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "global.h"

// init static vars
QTcpSocket* Global::socketServer = 0;
SQMPacketProcessor* Global::packetProcessor = 0;
SQMPacketHandler* Global::packetHandler = 0;
bool Global::init = false;

// Fixme: make it dynamic with a config file
QString Global::strServerHostname = "localhost";
quint16 Global::intServerPort = 1234;

void Global::initialize()
{
    // stop init process and exit if the class was allready initialized
    if(Global::init) {
        return;
    }

    // simplefy application instance
    QCoreApplication* app = QCoreApplication::instance();

    // init socket
    Global::socketServer = new QTcpSocket(app);

    // init packet handler
    SQMPacketHandler::create(app);
    Global::packetHandler = SQMPacketHandler::getInstance();
    Global::packetHandler->addDevice(Global::socketServer, false);

    // initialize packet processor, which process the packets
    Global::packetProcessor = new SQMPacketProcessor(Global::packetHandler);

    // connect packetHandler and packetProcessor
    app->connect(Global::packetHandler, SIGNAL(newPacketReceived(DataPacket*)), Global::packetProcessor, SLOT(newPacketReceived(DataPacket*)));

    // class was successfull initialized!
    Global::init = true;
}
