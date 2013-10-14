/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>

// own libs
#include "packetprocessor.h"
#include "protocol.pb.h"
#include "global.h"

class Chatmanager : public QObject
{
    Q_OBJECT
    public:
        Chatmanager(PacketProcessor* packetProcessor, QObject *parent = 0);

    private:
        PacketProcessor *packetProcessor;

    private slots:
        void handlePrivateChatMessage(EleaphRPCDataPacket* dataPacket);
};

#endif // CHATMANAGER_H
