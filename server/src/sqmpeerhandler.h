/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef SQMPEERHANDLER_H
#define SQMPEERHANDLER_H

// qt network libs
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

// qt core libs
#include <QtCore/QObject>
#include <QtCore/QList>

class SQMPeerHandler : public QObject
{
    Q_OBJECT
    signals:
        void newDevice(QIODevice *socket);
        void disconnectedDevice(QIODevice *socket);

    public:
        SQMPeerHandler(quint16 port, QHostAddress hostAddress = QHostAddress::Any, QObject *parent = 0);
        ~SQMPeerHandler();
        bool listen();

        // getter methods
        int getClientCount() { return this->lstClients.count(); }
        QTcpSocket* getClientbyIndex(int index) { return this->lstClients.at(index); }
        QHostAddress getHostAdress() { return this->hostAdress; }
        quint16 getPort() { return this->intPort; }
        bool isListening() { return this->serverListener->isListening(); }

    private:
        // server stuff
        QTcpServer *serverListener;
        QHostAddress hostAdress;
        quint16 intPort;

        // list stuff
        QList<QTcpSocket*> lstClients;

    private slots:
        void newConnection();
        void clientDisconnected();
};

#endif // SQMPEERHANDLER_H
