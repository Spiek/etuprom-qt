#include "sqmpacketprocessor.h"


//
// Section:
//  Con's and Decon's
//

SQMPacketProcessor::SQMPacketProcessor(QObject *parent) : QObject(parent)
{
    // register needed RPC methods
    EleaphProtoRPC *eleaphRPC = Global::getERPCInstance();
    eleaphRPC->registerRPCMethod("login", this, SLOT(handleLogin(DataPacket*)));
}

SQMPacketProcessor::~SQMPacketProcessor()
{

}


//
// Section:
//  Protocol handler methods
//

void SQMPacketProcessor::handleLogin(DataPacket* dataPacket)
{
    // simplefy login packet values
    Protocol::LoginRequest request;
    if(request.ParseFromArray(dataPacket->baRawPacketData, dataPacket->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse LoginRequest", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }
    QString strUsername = QString::fromStdString(request.username());
    QString strPassword  = QString::fromStdString(request.password());

    // search for user
    DatabaseHelper *databaseHelper = Global::getDatabaseHelper();
    Protocol::User* user = new Protocol::User;

    // inform the client if the user was found, or not
    Protocol::LoginResponse response;
    if(!databaseHelper->getUserByIdUserNameAndPw(strUsername, strPassword, user)) {
        response.set_type(Protocol::LoginResponse_Type_LoginIncorect);
    } else {
        response.set_type(Protocol::LoginResponse_Type_Success);
    }
    Global::getERPCInstance()->sendRPCDataPacket(dataPacket->ioPacketDevice, "login", response.SerializeAsString());

    // if login wasn't successfull, delte constructed user and end here
    if(response.type() == Protocol::LoginResponse_Type_LoginIncorect) {
        delete user;
        return;
    }

    // add user to the usermanager
    Global::getUserManager()->addUser(dataPacket->ioPacketDevice, user);

    // send the user information about the user who want to login
    Global::getERPCInstance()->sendRPCDataPacket(dataPacket->ioPacketDevice, "user", user->SerializeAsString());

    // get (if available) and send the contact list users to user
    Protocol::ContactList contactList;
    if(databaseHelper->getContactsByUserId(user->id(), &contactList)) {
        Global::getERPCInstance()->sendRPCDataPacket(dataPacket->ioPacketDevice, "contactlist", contactList.SerializeAsString());
    }
}

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
