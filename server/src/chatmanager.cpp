/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "chatmanager.h"

Chatmanager::Chatmanager(EleaphProtoRPC *eleaphRpc, Usermanager* managerUser, QObject *parent) : QObject(parent)
{
    // save the packet processor
    this->eleaphRpc = eleaphRpc;
    this->managerUser = managerUser;

    // protocol handlers
    this->eleaphRpc->registerRPCMethod(PACKET_DESCRIPTOR_CHAT_PRIVATE, this, SLOT(handlePrivateChatMessage(EleaphRPCDataPacket*)), false, EleaphProcessEvent_Before(managerUser));
}

void Chatmanager::handlePrivateChatMessage(EleaphRpcPacket dataPacket)
{
    // parse protocol
    Protocol::MessagePrivateClient message;
    if(!message.ParseFromArray(dataPacket.data()->baRawPacketData->constData(), dataPacket.data()->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse MessagePrivate", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }

    // get some needed data
    Usermanager *userManager = this->managerUser;
    QIODevice *deviceUser = dataPacket.data()->ioPacketDevice;
    EleaphProtoRPC *eleaphRpc = this->eleaphRpc;

    // if sended user is not logged in, then we have a very big protocol violation here!
    // FIXME: kill the peer!
    if(!userManager->isLoggedIn(deviceUser)) {
        return;
    }

    // get some needed informations
    Protocol::User* userSender = userManager->getConnectedUser(dataPacket.data()->ioPacketDevice);
    qint32 intUserIdReceiver = message.useridreceiver();
    quint32 intTimeStampCreated = QDateTime::currentMSecsSinceEpoch() / 1000;

    // send Protocol::MessagePrivateServer-message to the session and target user (if he is logged in)
    Protocol::MessagePrivateServer messageForClient;
    messageForClient.set_text(message.text());
    messageForClient.set_timestamp(intTimeStampCreated);

    // Private Message --> Src User Sessions (if available)
    messageForClient.set_direction(Protocol::MessagePrivateServer_Receiver_Session);
    messageForClient.set_userid(intUserIdReceiver);
    foreach(QIODevice *deviceSrcSession, userManager->getConnectedSessionSockets(userSender->id())) {
        // skip sending session (because session of course allready know the sent message :-))
        if(deviceSrcSession == deviceUser) {
            continue;
        }
        eleaphRpc->sendRPCDataPacket(deviceSrcSession, PACKET_DESCRIPTOR_CHAT_PRIVATE, messageForClient.SerializeAsString());
    }

    // Private Message --> Target User Sessions (if available)
    bool boolTransfered = userManager->isLoggedIn(intUserIdReceiver);
    if(boolTransfered) {
        messageForClient.set_direction(Protocol::MessagePrivateServer_Receiver_Target);
        messageForClient.set_userid(userSender->id());
        foreach(QIODevice *deviceTargetUser, userManager->getConnectedSessionSockets(intUserIdReceiver)) {
            eleaphRpc->sendRPCDataPacket(deviceTargetUser, PACKET_DESCRIPTOR_CHAT_PRIVATE, messageForClient.SerializeAsString());
        }
    }

    // save message in the database
    Global::getDatabaseHelper()->insertMessagePrivate(userSender->id(), intUserIdReceiver, QString::fromStdString(message.text()), boolTransfered, intTimeStampCreated);
}
