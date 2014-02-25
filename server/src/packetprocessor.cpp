/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#include "packetprocessor.h"


//
// Section:
//  Con's and Decon's
//

PacketProcessor::PacketProcessor(EleaphRpc *eleaphRpc, QObject *parent) : QObject(parent)
{
    // save eleaphRPC instance
    this->eleaphRpc = eleaphRpc;

    // construct sub manager
    this->managerUser = new Usermanager(eleaphRpc, this);
    this->managerChat = new Chatmanager(eleaphRpc, this->managerUser, this);
    this->managerContact = new Contactmanager(eleaphRpc, this->managerUser, this);
}

PacketProcessor::~PacketProcessor()
{
    delete this->managerUser;
    delete this->managerChat;
    delete this->managerContact;
}

EleaphRpc* PacketProcessor::getEleaphRpc()
{
    return this->eleaphRpc;
}

Usermanager* PacketProcessor::getUserManager()
{
    return this->managerUser;
}

Chatmanager* PacketProcessor::getChatManager()
{
    return this->managerChat;
}


Contactmanager* PacketProcessor::getContactManager()
{
    return this->managerContact;
}
