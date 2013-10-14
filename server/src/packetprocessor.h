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

// protocol manager's
#include "usermanager.h"
#include "chatmanager.h"
#include "contactmanager.h"

// forward declaration, becuase of cyrcle including of the sub managers
// (every submanager need a pointer to the PacketProcessor to access ressources)
class Usermanager;
class Chatmanager;
class Contactmanager;


//
// Packet Descriptors
//
// here we define the Protocol Descriptors for all packet types which will be used in PacketProcessor and it's Modules
//

// User Module
#define PACKET_DESCRIPTOR_USER_LOGIN "user.login"
#define PACKET_DESCRIPTOR_USER_LOGOUT "user.logout"
#define PACKET_DESCRIPTOR_USER_SELF_GET_INFO "user.self.getinfo"

// Contact Module (require User Module)
#define PACKET_DESCRIPTOR_CONTACT_GET_LIST "contact.getlist"
#define PACKET_DESCRIPTOR_CONTACT_ALTERED "contact.altered"

// Chatmanager Module
#define PACKET_DESCRIPTOR_CHAT_PRIVATE "chat.private"

class PacketProcessor : public QObject
{
    Q_OBJECT
    public:
        // con/decon
        PacketProcessor(EleaphProtoRPC* eleaphRpc, QObject *parent = 0);
        ~PacketProcessor();
        EleaphProtoRPC* getEleaphRpc();
        Usermanager* getUserManager();
        Contactmanager* getContactManager();

    private:
        // sub protocol handlers
        Usermanager *managerUser;
        Chatmanager *managerChat;
        Contactmanager* managerContact;

        // helper methods
        EleaphProtoRPC *eleaphRpc;
};

#endif // SQMPACKETPROCESSOR_H
