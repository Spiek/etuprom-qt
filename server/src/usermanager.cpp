#include "usermanager.h"

Usermanager::Usermanager(QObject *parent) : QObject(parent)
{
    // handle client disconnects
    this->connect(Global::getERPCInstance(), SIGNAL(sigDeviceRemoved(QIODevice*)), this, SLOT(handle_client_disconnect(QIODevice*)));
}

Usermanager::~Usermanager()
{

}




//
// Section:
//  Protocol helper methods
//

void Usermanager::userChanged(qint32 userid, bool refreshUser)
{
    return this->userChanged(this->getConnectedDevice(userid), refreshUser);
}

void Usermanager::userChanged(QIODevice *device, bool refreshUser)
{
    // if we have a user change on a non logged in user, exit
    Protocol::User* userChanged = this->getConnectedUser(device);
    if(!userChanged) {
        return;
    }

    // refresh user if wanted
    if(refreshUser) {
        this->refreshUser(userChanged);
    }

    // select all users from changed user's contact list which are online
    Protocol::Users users;

    // if no contact in contact list of changed user is online, don't inform anyone
    if(!Global::getDatabaseHelper()->getOnlineContactsByUserId(userChanged->id(), &users)) {
        return;
    }

    // ... otherwise inform all online contacts of the altered user (one by one) about the change
    for(int i = 0; i < users.user_size(); i++) {
        // extract needed values from protobuf message
        qint32 intUserIdOfUserToInform = users.mutable_user(i)->id();

        // get device of conntected user, and skip him if he isn't really contected (if no device was found!)
        QIODevice *deviceOfUserToInform = this->getConnectedDevice(intUserIdOfUserToInform);
        if(!deviceOfUserToInform) {
            continue;
        }

        // inform connected user in contactlist of changed user, about the change
        Global::getERPCInstance()->sendRPCDataPacket(deviceOfUserToInform, "user_altered", userChanged->SerializeAsString());
    }
}


//
// Section:
//  User managment helper methods
//

void Usermanager::addUser(QIODevice *device, Protocol::User *user)
{
    // exit if the user does exist or the user is invalid
    if(!user || this->mapIdUser.contains(user->id())) {
        return;
    }

    // add to socket -> user map
    this->mapSocketUser.insert(device, user);

    // add to userid -> QPair(device, user) map
    this->mapIdUser.insert(user->id(), QPair<QIODevice*, Protocol::User*>(device, user));

    // update the user in database
    user->set_online(true);
    Global::getDatabaseHelper()->updateUserOnlineStateById(user, true);

    // ... and inform all users which belongs to the user's contacht list about users state change
    this->userChanged(user->id(), false);
}

void Usermanager::removeUser(QIODevice *device)
{
    // take from socket -> user map
    Protocol::User *user = this->mapSocketUser.value(device);

    // call overloaded method
    return this->removeUser(user);
}

void Usermanager::removeUser(Protocol::User *user)
{
    // exit if the user doesn't exist or the user is invalid
    if(!user || !this->mapIdUser.contains(user->id())) {
        return;
    }

    // update the user in database
    user->set_online(false);
    Global::getDatabaseHelper()->updateUserOnlineStateById(user, false);

    // ... and inform all users which belongs to the user's contacht list about users state change
    this->userChanged(user->id(), false);

    // REMOVE USER

    // remove from userid -> QPair(device, user) map
    QPair<QIODevice*, Protocol::User*> pairValueUser = this->mapIdUser.take(user->id());

    // remove from socket -> user map
    this->mapSocketUser.take(pairValueUser.first);

    // delete user
    delete pairValueUser.second;
}

bool Usermanager::isLoggedIn(QIODevice *device)
{
    return (device ? this->mapSocketUser.contains(device) : false);
}

bool Usermanager::isLoggedIn(qint32 userID)
{
    return this->mapIdUser.contains(userID);
}

bool Usermanager::refreshUser(Protocol::User *user)
{
    return Global::getDatabaseHelper()->getUserById(user->id(), user);
}

Protocol::User* Usermanager::getConnectedUser(QIODevice *device)
{
    return this->mapSocketUser.value(device, (Protocol::User*)0);
}

Protocol::User* Usermanager::getConnectedUser(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>((QIODevice*)0, (Protocol::User*)0));
    return pairSocketUser.second;
}

QIODevice* Usermanager::getConnectedDevice(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>((QIODevice*)0, (Protocol::User*)0));
    return pairSocketUser.first;
}

//
// Socket Signal handlings
//

void Usermanager::handle_client_disconnect(QIODevice *device)
{
    // if the given device is logged in, log the device/user off
    if(this->mapSocketUser.contains(device)) {
        this->removeUser(device);
    }
}
