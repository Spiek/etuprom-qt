#include "usermanager.h"

//
// Section:
//  Protocol helper methods
//

void Usermanager::userChanged(qint32 userid)
{
    return this->userChanged(this->getConnectedDevice(userid));
}

void Usermanager::userChanged(QIODevice *device)
{
    // if we have a user change on a non logged in user, exit
    Protocol::User* userChanged = this->getConnectedUser(device);
    if(!userChanged) {
        return;
    }

    // refresh user
    this->refreshUser(userChanged);

    // build User Altered protocol packet and serialize it
    Protocol::Packet packetUserChange;
    packetUserChange.set_packettype(Protocol::Packet_PacketType_UserAltered);
    packetUserChange.mutable_useraltered()->MergeFrom(*userChanged);
    std::string strSerialzedPacket = packetUserChange.SerializeAsString();

    // select all users from changed user's contact list which are online and inform them about the change
    QSqlQuery query = DatabaseHelper::getOnlineContactsByUserId(userChanged->id());
    while(query.next()) {
        // get conntected user by id, and skip him if he isn't really contected
        Protocol::User *userOfContactList = this->getConnectedUser(query.value(0).toInt());
        if(!userOfContactList) {
            continue;
        }

        // inform user about the change
        QIODevice *device = this->getConnectedDevice(userOfContactList->id());

        // inform user about the change
        SQMPacketHandler::getInstance()->sendDataPacket(device, strSerialzedPacket);
    }
}

//
// Section:
//  Database helper methods
//

Protocol::User* Usermanager::setUserfromQuery(QSqlQuery *query,  Protocol::User* user)
{
    // construct target user if doesn't exist
    if(!user) {
        user = new Protocol::User;
    }

    // set the values from the query
    user->set_id(query->value(0).toInt());
    user->set_username(query->value(1).toString().toStdString());
    user->set_state(query->value(2).toInt());
    user->set_online(query->value(3).toBool());
    user->set_visible(query->value(4).toBool());

    // return new setted user
    return user;
}


//
// Section:
//  User managment helper methods
//

void Usermanager::addUser(QIODevice *device, Protocol::User *user)
{
    // add to socket -> user map
    this->mapSocketUser.insert(device, user);

    // add to userid -> QPair(device, user) map
    this->mapIdUser.insert(user->id(), QPair<QIODevice*, Protocol::User*>(device, user));

    // update the user (in database and inform all users which belongs to the user about users state change)
    DatabaseHelper::updateUserOnlineState(user->id(), true);
    this->userChanged(user->id());
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

    // update the user (in database and inform all users which belongs to the user about users state change)
    DatabaseHelper::updateUserOnlineState(user->id(), false);
    this->userChanged(user->id());

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

Protocol::User* Usermanager::refreshUser(Protocol::User *user)
{
    QSqlQuery queryUserRefresh = DatabaseHelper::getUserById(user->id());
    queryUserRefresh.next();
    return this->setUserfromQuery(&queryUserRefresh, user);
}

Protocol::User* Usermanager::getConnectedUser(QIODevice *device)
{
    return this->mapSocketUser.value(device, 0);
}

Protocol::User* Usermanager::getConnectedUser(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>(0, 0));
    return pairSocketUser.second;
}

QIODevice* Usermanager::getConnectedDevice(qint32 userid)
{
    QPair<QIODevice*, Protocol::User*> pairSocketUser = this->mapIdUser.value(userid, QPair<QIODevice*, Protocol::User*>(0, 0));
    return pairSocketUser.first;
}


//
// Section:
//  SINGELTON
//
Usermanager::Usermanager(QObject *parent) : QObject(parent)
{

}

Usermanager::~Usermanager()
{

}

Usermanager* Usermanager::userManager;

void Usermanager::create(QObject *parent)
{
    // if singelton class wasn't constructed yet, construct it
    if(!Usermanager::userManager) {
        Usermanager::userManager = new Usermanager(parent);
    }
}

Usermanager* Usermanager::getInstance()
{
    return Usermanager::userManager;
}
