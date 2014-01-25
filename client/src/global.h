/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef GLOBAL_H
#define GLOBAL_H

// Qt (core)
#include <QtWidgets/QApplication>
#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtCore/QFileInfo>

// Qt (network)
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostInfo>

// own (collective)
#include "EleaphProtoRpc"
#include "protocol.pb.h"

class Global
{
    public:
        static QTcpSocket *socketServer;
        static EleaphProtoRPC *eleaphRpc;
        static QString strServerHostname;
        static quint16 intServerPort;
        static bool boolLoggedIn;
        static Protocol::User* user;
        static QString strSessionName;

        // protocol implementations
        static QMap<qint32, Protocol::Contact*> mapContactList;
        static QMap<qint32, Protocol::User*> mapCachedUsers;

        static void initialize();

    private:
        // settings
        static QSettings *settings;
        static bool init;
};

#endif // GLOBAL_H

