/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef SQMPACKETPROCESSOR_H
#define SQMPACKETPROCESSOR_H

// Qt (core)
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QIODevice>
#include <QtCore/QPair>

// own libs
#include "EleaphRpc"
#include "global.h"
#include "collective/proto/packettypes.h"

// protocol manager's
#include "usermanager.h"
#include "chatmanager.h"
#include "contactmanager.h"

class PacketProcessor : public QObject
{
    Q_OBJECT
    public slots:
        void start();

    public:
        // con/decon
        PacketProcessor(EleaphRpc* eleaphRpc, QObject *parent = 0);
        ~PacketProcessor();
        EleaphRpc* getEleaphRpc();

        // manager getter
        Usermanager* getUserManager();
        Chatmanager* getChatManager();
        Contactmanager* getContactManager();

    private:
        // sub protocol handlers
        Usermanager *managerUser;
        Chatmanager *managerChat;
        Contactmanager* managerContact;

        // helper methods
        EleaphRpc *eleaphRpc;
};

#endif // SQMPACKETPROCESSOR_H
