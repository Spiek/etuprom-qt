/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#include "sqmpacketprocessor.h"


//
// Section:
//  Con's and Decon's
//

SQMPacketProcessor::SQMPacketProcessor(QObject *parent) : QObject(parent)
{
    // construct user manager
    this->managerUser = new Usermanager(Global::getERPCInstance(), this);

    // register needed RPC methods
    //EleaphProtoRPC *eleaphRPC = Global::getERPCInstance();

}

SQMPacketProcessor::~SQMPacketProcessor()
{
    delete this->managerUser;
}


//
// Section:
//  Protocol handler methods
//



void SQMPacketProcessor::handleUserMessage(DataPacket *dataPacket)
{
    // simplefy global values
    /*Usermanager *userManager = Usermanager::getInstance();
    Protocol::User *user = userManager->getConnectedUser(dataPacket->ioPacketDevice);
    qint32 intSenderUserId = user->id();

    // simplefy UserMessage values
    Protocol::UserMessage *userMessage = protocolPacket->mutable_usermessage();
    QString strMessage = QString::fromStdString(userMessage->messagetext());
    qint32 intReceiverUserId = userMessage->receiveruserid();

    // set sender user_id
    userMessage->set_senderuserid(intSenderUserId);

    // send the message directly to user if user is online
    if(userManager->isLoggedIn(intReceiverUserId)) {
        QIODevice *deviceUserReceiver = userManager->getConnectedDevice(intReceiverUserId);
        SQMPacketHandler::getInstance()->sendDataPacket(deviceUserReceiver, protocolPacket->SerializeAsString());
    }

    // save message in database
    DatabaseHelper::createNewUserMessage(intSenderUserId, intReceiverUserId, strMessage);*/
}
