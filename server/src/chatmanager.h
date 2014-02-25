/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>

// own libs
#include "collective/proto/packettypes.h"
#include "EleaphProtoRpc"
#include "protocol.pb.h"
#include "global.h"

// sub module dependings
#include "usermanager.h"

class Chatmanager : public QObject
{
    Q_OBJECT
    public:
        Chatmanager(EleaphRpc *eleaphRpc, Usermanager* managerUser, QObject *parent = 0);

    private:
        EleaphRpc *eleaphRpc;
        Usermanager *managerUser;

    private slots:
        void handlePrivateChatMessage(EleaphRpcPacket dataPacket);
};

#endif // CHATMANAGER_H
