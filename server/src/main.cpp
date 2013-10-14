/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

// Qt (core)
#include <QtCore/QCoreApplication>

// Qt (sql)
#include <QtSql/QSqlDatabase>

// own libs
#include "global.h"
#include "packetprocessor.h"

// collective own libs
#include "EleaphProtoRpc"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // set some program values
    a.setApplicationName("SQMServer");

    // initialize globalization
    Global::initialize();

    // initialize eleaph-proto-RPC-System
    EleaphProtoRPC *eleaphRPC = new EleaphProtoRPC(&a, 65536);

    // initialize packet processor, which process the packets
    // (we are don't do anything with the object, but construct, everything is handled in the constructor!)
    PacketProcessor* packetProcessor = new PacketProcessor(eleaphRPC, &a);
    Q_UNUSED(packetProcessor);

    // start the tcp listening
    eleaphRPC->startTcpListening(Global::getConfigValue("server/port", 0).toInt());

    // start eventloop
    return a.exec();
}
