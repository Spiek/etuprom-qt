/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "chatmanager.h"

Chatmanager::Chatmanager(PacketProcessor* packetProcessor, QObject *parent) : QObject(parent)
{
    // save the packet processor
    this->packetProcessor = packetProcessor;

    // protocol handlers
    EleaphProtoRPC *eleaphRpc = packetProcessor->getEleaphRpc();
    eleaphRpc->registerRPCMethod("message.private", this, SLOT(handlePrivateChatMessage(DataPacket*)));
}

void Chatmanager::handlePrivateChatMessage(DataPacket *dataPacket)
{
    // parse protocol
    Protocol::MessagePrivateClient message;
    if(!message.ParseFromArray(dataPacket->baRawPacketData->constData(), dataPacket->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse MessagePrivate", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }

    // get some needed data
    Usermanager *userManager = this->packetProcessor->getUserManager();
    QIODevice *deviceUser = dataPacket->ioPacketDevice;
    EleaphProtoRPC *eleaphRpc = this->packetProcessor->getEleaphRpc();

    // if sended user is not logged in, then we have a very big protocol violation here!
    // FIXME: kill the peer!
    if(!userManager->isLoggedIn(deviceUser)) {
        return;
    }

    // get some needed informations
    Protocol::User* userSender = userManager->getConnectedUser(dataPacket->ioPacketDevice);
    qint32 intUserIdReceiver = message.useridreceiver();
    quint32 intTimeStampCreated = QDateTime::currentMSecsSinceEpoch() / 1000;
    bool boolTransfered = false;

    // send Protocol::MessagePrivateServer-message to the target user (if he is logged in)
    if(userManager->isLoggedIn(intUserIdReceiver)) {
        Protocol::MessagePrivateServer messageForClient;
        messageForClient.mutable_usersender()->MergeFrom(*userSender);
        messageForClient.set_text(message.text());
        messageForClient.set_timestamp(intTimeStampCreated);

        // send Private Message packet to the target user
        QIODevice *deviceTargetUser = userManager->getConnectedDevice(intUserIdReceiver);
        eleaphRpc->sendRPCDataPacket(deviceTargetUser, "message.private", messageForClient.SerializeAsString());
        boolTransfered = true;
    }

    // save message in the database
    Global::getDatabaseHelper()->insertMessagePrivate(userSender->id(), intUserIdReceiver, QString::fromStdString(message.text()), boolTransfered, intTimeStampCreated);
}
