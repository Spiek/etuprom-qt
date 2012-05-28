/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "SQMPeerHandler"

//
// Con's and Decon
//

SQMPeerHandler::SQMPeerHandler(quint16 port, QHostAddress hostAddress, QObject *parent) : QObject(parent)
{
    // save parameters as properties
    this->intPort = port;
    this->hostAdress = hostAddress;
    this->serverListener = new QTcpServer(this);
}

SQMPeerHandler::~SQMPeerHandler()
{
    // clean up
    delete this->serverListener;
}


//
// Public Functions
//

/*
 * listen - start tcp listener
 */
bool SQMPeerHandler::listen()
{
    // start server listener and install signal-slot connection for new connections
    this->connect(this->serverListener, SIGNAL(newConnection()), this, SLOT(newConnection()));
    return this->serverListener->listen(this->hostAdress, this->intPort);
}


//
// Server Signal handlings!
//  - all methods in this section are called exclusive from Qt's event System!
//    IMPORTANT: don't call these methods directly!
//

/*
 * newClient - handles new client connections from the TcpServer
 */
void SQMPeerHandler::newConnection()
{
    // get tcpSocket of new Client and ADD him to the client list,
    // so that we are able later to track which clients are connected
    QTcpSocket *socketClient = this->serverListener->nextPendingConnection();
    this->lstClients.append(socketClient);

    // forward IODEVICE for external use
    emit this->newDevice(socketClient);

    // handle the disconnect signal
    this->connect(socketClient, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
}


//
// Client Signal handlings!
//  - all methods in this section are called exclusive from Qt's event System!
//    IMPORTANT: don't call these methods directly!
//

/*
 * clientDisconnected - handles disconnect events from the client
 */
void SQMPeerHandler::clientDisconnected()
{
    // get tcpSocket of new Client and REMOVE him from the client list
    QTcpSocket *socketClient = qobject_cast<QTcpSocket*>(this->sender());
    this->lstClients.removeOne(socketClient);

    // tell the world outside that the socket is not available anymore
    emit this->disconnectedDevice(socketClient);
}
