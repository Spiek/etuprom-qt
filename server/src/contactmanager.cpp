#include "contactmanager.h"

Contactmanager::Contactmanager(EleaphProtoRPC *eleaphRPC, Usermanager *managerUser, QObject *parent) : QObject(parent)
{
    // save eleaphrpc
    this->eleaphRPC = eleaphRPC;
    this->managerUser = managerUser;

    // register all needed protocol messages
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_CONTACT_GET_LIST, this, SLOT(handleContactList(EleaphRPCDataPacket*)));

    // connect all needed signals from other modules
    this->connect(managerUser, SIGNAL(sigUserChanged(Usermanager::UserShared,QIODevice*,Usermanager::UserChangeType)), this, SLOT(handleContactChange(Usermanager::UserShared)));
}

void Contactmanager::handleContactList(EleaphRpcPacket dataPacket)
{
    // get the logged in user, if the given user is not logged in, don't handle the packet
    Protocol::User *user = this->managerUser->getConnectedUser(dataPacket.data()->ioPacketDevice);
    if(!user) {
        return;
    }

    // simplefy object access
    DatabaseHelper *databaseHelper = Global::getDatabaseHelper();

    // get (if available) all contacts from logged in user and send the list to the logged in user, otherwise send empty packet
    Protocol::ContactList contactList;
    if(databaseHelper->getContactsByUserId(user->id(), &contactList)) {
        this->eleaphRPC->sendRPCDataPacket(dataPacket.data()->ioPacketDevice, PACKET_DESCRIPTOR_CONTACT_GET_LIST, contactList.SerializeAsString());
    } else {
        this->eleaphRPC->sendRPCDataPacket(dataPacket.data()->ioPacketDevice, PACKET_DESCRIPTOR_CONTACT_GET_LIST);
    }
}

void Contactmanager::handleContactChange(Usermanager::UserShared userChanged)
{
    // select all users from changed user's contact list which are online
    Protocol::Users users;

    // if no contact in contact list of changed user is online, don't inform anyone
    if(!Global::getDatabaseHelper()->getAllOnlineContactsByUserId(userChanged.data()->id(), &users)) {
        return;
    }

    // ... otherwise inform all online contacts of the altered user (one by one) about the change
    for(int i = 0; i < users.user_size(); i++) {
        // extract needed values from protobuf message
        qint32 intUserIdOfUserToInform = users.mutable_user(i)->id();

        // inform all sessions of online "contact list"-users about the user change
        foreach(QIODevice *deviceOfUserToInform, this->managerUser->getConnectedSessions(intUserIdOfUserToInform)) {
            this->eleaphRPC->sendRPCDataPacket(deviceOfUserToInform, PACKET_DESCRIPTOR_CONTACT_ALTERED, userChanged.data()->SerializeAsString());
        }
    }
}
