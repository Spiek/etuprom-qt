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
    Protocol::MessagePrivate message;
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
    qint32 intUserIdSender = userManager->getConnectedUser(dataPacket->ioPacketDevice)->id();
    qint32 intUserIdReceiver = message.useridsenderreceiver();
    QString strText = QString::fromStdString(message.text());
    quint32 intTimeStampCreated = message.timestamp();
    bool boolTransfered = false;

    // if timestamp is in the past we have a Protocol Violation
    if(intTimeStampCreated < (QDateTime::currentMSecsSinceEpoch() / 1000)) {
        qWarning("[%s][%d] - Protocol Violation by Timestamp check (timestamp is in the past!)", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }

    // set userSenderReceiver to sending user, so that receiver knows who has send the message
    message.set_useridsenderreceiver(intUserIdSender);

    // send the user message to the target user (if he is logged in)
    if(userManager->isLoggedIn(intUserIdReceiver)) {
        QIODevice *deviceTargetUser = userManager->getConnectedDevice(intUserIdReceiver);
        eleaphRpc->sendRPCDataPacket(deviceTargetUser, "message.private", message.SerializeAsString());
        boolTransfered = true;
    }

    // save message in the database
    Global::getDatabaseHelper()->insertMessagePrivate(intUserIdSender, intUserIdReceiver, strText, boolTransfered, message.timestamp());
}
