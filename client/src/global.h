/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef GLOBAL_H
#define GLOBAL_H

// Qt (core)
#include <QtGui/QApplication>

// Qt (network)
#include <QtNetwork/QTcpSocket>

// own (collective)
#include "EleaphProtoRpc"

class Global
{
    public:
        static QTcpSocket *socketServer;
        static EleaphProtoRPC *eleaphRpc;
        static QString strServerHostname;
        static quint16 intServerPort;
        static bool boolLoggedIn;

        static void initialize();

    private:
        static bool init;
};

#endif // GLOBAL_H
