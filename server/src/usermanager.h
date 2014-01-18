/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#ifndef USERMANAGER_H
#define USERMANAGER_H

// Qt (core)
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QIODevice>
#include <QtCore/QPair>
#include <QtCore/QVariant>
#include <QtCore/QSharedPointer>

// Qt (sql)
#include <QtSql/QSqlQuery>

// own libs
#include "collective/proto/packettypes.h"
#include "EleaphProtoRpc"
#include "protocol.pb.h"
#include "global.h"

class Usermanager : public QObject
{
    Q_OBJECT
    public:
        typedef QSharedPointer<Protocol::User> UserShared;
        enum class UserChangeType : quint8 {
            // if first session will be creation "UserAdded" will be set too
            UserAdded           = 1,
            UserSessionAdded    = 2,

            // if last session will be deleted "UserRemoved" will be set too
            UserRemoved         = 4,
            UserSessionRemoved  = 8
        };

    signals:
        void sigUserChanged(Usermanager::UserShared userChanged, QIODevice *deviceProducerOfChange, Usermanager::UserChangeType changeType);

    public:
        // con and decon
        Usermanager(EleaphProtoRPC *eleaphRPC, QObject *parent = 0);
        ~Usermanager();

        // user managment helper methods
        void addUserSession(QIODevice *device, Protocol::User *user);
        void removeUserSession(QIODevice *device);
        bool isLoggedIn(QIODevice *device);
        bool isLoggedIn(qint32 userID);
        Protocol::User* getConnectedUser(QIODevice *device);
        Protocol::User* getConnectedUser(qint32 userid);
        QIODevice* getConnectedDevice(qint32 userid);

        // settings setter/getter
        void setSettingsActivateMultiSessions(bool enabled);
        bool getSettingsActivateMultiSession();

    private:
        QMap<QIODevice*, Protocol::User*> mapSocketsUser;
        QMap<qint32, QMap<QIODevice*, Protocol::User*>* > mapUsersSessions;
        EleaphProtoRPC* eleaphRPC;

        // settings
        bool boolSettingsMultiSessionsActive;

    private slots:
        void handleUserChange(Usermanager::UserShared userChanged, QIODevice *deviceProducerOfChange, Usermanager::UserChangeType changeType);
        void handle_client_disconnect(QIODevice *device);
        void handleLogin(EleaphRpcPacket dataPacket);
        void handleLogout(EleaphRpcPacket dataPacket);
        void handleUserInfoSelf(EleaphRpcPacket dataPacket);
};

#endif // USERMANAGER_H
