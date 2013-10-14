#include "contactmanager.h"

Contactmanager::Contactmanager(PacketProcessor *packetProcessor, QObject *parent) : QObject(parent)
{
    // save eleaphrpc
    this->packetProcessor = packetProcessor;

    // register all needed protocol messages
    EleaphProtoRPC* eleaphRpc = packetProcessor->getEleaphRpc();
    eleaphRpc->registerRPCMethod(PACKET_DESCRIPTOR_CONTACT_GET_LIST, this, SLOT(handleContactList(EleaphRPCDataPacket*)));

    // connect all needed signals from other modules
    this->connect(this->packetProcessor->getUserManager(), SIGNAL(sigUserChanged(Protocol::User*,QIODevice*,bool)), this, SLOT(handleContactChange(Protocol::User*,QIODevice*)));
}

void Contactmanager::handleContactList(EleaphRPCDataPacket *dataPacket)
{
    // init Datapacket Releaser
    volatile DataPacketDeallocater datapacketDeallocator(dataPacket);

    // get the logged in user, if the given user is not logged in, don't handle the packet
    Protocol::User *user = this->packetProcessor->getUserManager()->getConnectedUser(dataPacket->ioPacketDevice);
    if(!user) {
        return;
    }

    // simplefy object access
    DatabaseHelper *databaseHelper = Global::getDatabaseHelper();

    // get (if available) all contacts from logged in user and send the list to the logged in user, otherwise send empty packet
    Protocol::ContactList contactList;
    if(databaseHelper->getContactsByUserId(user->id(), &contactList)) {
        this->packetProcessor->getEleaphRpc()->sendRPCDataPacket(dataPacket->ioPacketDevice, PACKET_DESCRIPTOR_CONTACT_GET_LIST, contactList.SerializeAsString());
    } else {
        this->packetProcessor->getEleaphRpc()->sendRPCDataPacket(dataPacket->ioPacketDevice, PACKET_DESCRIPTOR_CONTACT_GET_LIST);
    }
}

void Contactmanager::handleContactChange(Protocol::User* userChanged, QIODevice* deviceProducerOfChange)
{
    // select all users from changed user's contact list which are online
    Protocol::Users users;

    // if no contact in contact list of changed user is online, don't inform anyone
    if(!Global::getDatabaseHelper()->getAllOnlineContactsByUserId(userChanged->id(), &users)) {
        return;
    }

    // ... otherwise inform all online contacts of the altered user (one by one) about the change
    for(int i = 0; i < users.user_size(); i++) {
        // extract needed values from protobuf message
        qint32 intUserIdOfUserToInform = users.mutable_user(i)->id();

        // get device of conntected user, and skip him if he isn't really contected (if no device was found!)
        QIODevice *deviceOfUserToInform = this->packetProcessor->getUserManager()->getConnectedDevice(intUserIdOfUserToInform);
        if(!deviceOfUserToInform || deviceOfUserToInform == deviceProducerOfChange) {
            continue;
        }

        // inform connected user in contactlist of changed user, about the change
        this->packetProcessor->getEleaphRpc()->sendRPCDataPacket(deviceOfUserToInform, PACKET_DESCRIPTOR_CONTACT_ALTERED, userChanged->SerializeAsString());
    }
}
