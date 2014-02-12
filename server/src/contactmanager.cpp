#include "contactmanager.h"

Contactmanager::Contactmanager(EleaphProtoRPC *eleaphRPC, Usermanager *managerUser, QObject *parent) : QObject(parent)
{
    // save eleaphrpc
    this->eleaphRPC = eleaphRPC;
    this->managerUser = managerUser;

    // register all needed protocol messages
    eleaphRPC->registerRPCMethod(PACKET_DESCRIPTOR_CONTACT_GET_LIST, this, SLOT(handleContactList(EleaphRPCDataPacket*)), false, EleaphProcessEvent_Before(managerUser));

    // connect all needed signals from other modules
    this->connect(managerUser, SIGNAL(sigUserChanged(Usermanager::SharedSession,QIODevice*,Usermanager::UserChangeType)), this, SLOT(handleContactChange(Usermanager::SharedSession)));
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

    // add all user sessions as virtual users!
    databaseHelper->getContactsByUserId(user->id(), &contactList);
    foreach(QIODevice* deviceSession, this->managerUser->getConnectedSessionSockets(user->id())) {
        Protocol::Contact *contact = contactList.add_contact();
        contact->mutable_user()->MergeFrom(*user);
        contact->mutable_user()->set_username(this->managerUser->getConnectedSession(deviceSession)->sessionname());
        contact->set_group("Sessions");
    }


    // send contactlist or if contactlist is empty, a simple empty content
    this->eleaphRPC->sendRPCDataPacket(dataPacket.data()->ioPacketDevice, PACKET_DESCRIPTOR_CONTACT_GET_LIST, contactList.contact_size() > 0 ? contactList.SerializeAsString() : "");
}

void Contactmanager::handleContactChange(Usermanager::SharedSession session)
{
    // select all users from changed user's contact list which are online
    Protocol::Users users;

    // if no contact in contact list of changed user is online, don't inform anyone
    if(!Global::getDatabaseHelper()->getAllOnlineContactsByUserId(session.data()->mutable_user()->id(), &users)) {
        return;
    }

    // ... otherwise inform all online contacts of the altered user (one by one) about the change
    for(int i = 0; i < users.user_size(); i++) {
        // extract needed values from protobuf message
        qint32 intUserIdOfUserToInform = users.mutable_user(i)->id();

        // inform all sessions of online "contact list"-users about the user change
        foreach(QIODevice *deviceOfUserToInform, this->managerUser->getConnectedSessionSockets(intUserIdOfUserToInform)) {
            this->eleaphRPC->sendRPCDataPacket(deviceOfUserToInform, PACKET_DESCRIPTOR_CONTACT_ALTERED, session.data()->SerializeAsString());
        }
    }
}
