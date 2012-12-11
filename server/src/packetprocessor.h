/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#ifndef SQMPACKETPROCESSOR_H
#define SQMPACKETPROCESSOR_H

// forward declaration
class PacketProcessor;

// Qt (core)
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QIODevice>
#include <QtCore/QPair>

// own libs
#include "EleaphProtoRpc"
#include "global.h"

// sub manager
#include "usermanager.h"
#include "chatmanager.h"

// forward delclarion, becuase of cyrcle including of the usermanager!
class Usermanager;
class Chatmanager;

class PacketProcessor : public QObject
{
    Q_OBJECT
    public:
        // con/decon
        PacketProcessor(QObject *parent = 0);
        ~PacketProcessor();
        EleaphProtoRPC* getEleaphRpc();
        Usermanager* getUserManager();

    private:
        // sub protocol handlers
        Usermanager *managerUser;
        Chatmanager *managerChat;

        // helper methods
        EleaphProtoRPC *eleaphRpc;
};

#endif // SQMPACKETPROCESSOR_H
