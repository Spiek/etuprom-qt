/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "global.h"

// init static vars
QTcpSocket* Global::socketServer = 0;
SQMPacketProcessor* Global::packetProcessor = 0;
EleaphProtoRPC* Global::eleaphRpc = 0;
bool Global::init = false;

// init forms
MainWindow* Global::formMain = 0;

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
    QCoreApplication* app = QApplication::instance();

    // init socket
    Global::socketServer = new QTcpSocket(app);

    // init packet handler
    Global::eleaphRpc = new EleaphProtoRPC(app);
    Global::eleaphRpc->addDevice(Global::socketServer, IEleaph::NeverForgetDevice);

    // initialize packet processor, which process the packets
    Global::packetProcessor = new SQMPacketProcessor(app);

    // init main Window
    Global::formMain = new MainWindow;

    // class was successfull initialized!
    Global::init = true;
}
