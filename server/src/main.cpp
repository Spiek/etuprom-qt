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

    // initialize peer handler, which handled the peers
    SQMPeerHandler peerHandler(1234, QHostAddress::Any, &a);
    peerHandler.listen();

    // initialize packet handler, which handled the packet parsing
    SQMPacketHandler::create(&a);
    SQMPacketHandler* packetHandler = SQMPacketHandler::getInstance();
    a.connect(&peerHandler, SIGNAL(newDevice(QIODevice*)), packetHandler, SLOT(newDevice(QIODevice*)));
    a.connect(&peerHandler, SIGNAL(disconnectedDevice(QIODevice*)), packetHandler, SLOT(disconnectedDevice(QIODevice*)));

    // start eventloop
    return a.exec();
}
