/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

// qt core libs
#include <QtCore/QCoreApplication>

// own libs
#include "sqmpeerhandler.h"
#include "sqmpackethandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // initialize peer and packet handler
    SQMPeerHandler peerHandler(1234, QHostAddress::Any, &a);
    peerHandler.listen();

    SQMPacketHandler packetHandler(&a);
    a.connect(&peerHandler, SIGNAL(newDevice(QIODevice*)), &packetHandler, SLOT(newDevice(QIODevice*)));

    return a.exec();
}
