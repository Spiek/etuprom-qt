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
#include "EleaphRpc"
#include "protocol.pb.h"
#include "global.h"

class Usermanager : public QObject
{
    Q_OBJECT
    public:
        typedef QSharedPointer<Protocol::Session> SharedSession;

        // UserChangeType
        // - represent the state of the User
        // - will be used for external communication over "sigUserChanged"-SIGNAL
        //
        // explanation:
        // UserAdded            (flag)  -> will be set on first login
        // UserSessionAdded     (flag)  -> will be set on every login
        // UserRemoved          (flag)  -> will be set on last logout
        // UserSessionRemoved   (flag)  -> will be set on every logout
        //
        enum class UserChangeType : quint8 {
            UserAdded           = 1,
            UserSessionAdded    = 2,
            UserRemoved         = 4,
            UserSessionRemoved  = 8
        };

    signals:
        void sigUserChanged(Usermanager::SharedSession userSession, QIODevice *deviceProducerOfChange, Usermanager::UserChangeType changeType);

    public:
        // Con and decon
        Usermanager(EleaphRpc *eleaphRPC, QObject *parent = 0);
        ~Usermanager();

        // External user managment helper methods
        void addUserSession(QIODevice *device, Protocol::Session *session);
        void removeUserSession(QIODevice *device);
        bool isLoggedIn(QIODevice *device);
        bool isLoggedIn(qint32 userID);
        Protocol::Session*                     getConnectedSession(QIODevice *device);
        QList<Protocol::Session*>              getConnectedSessions(qint32 userid);
        QList<QIODevice*>                      getConnectedSessionSockets(qint32 userid);
        Protocol::User*                        getConnectedUser(QIODevice *device);

        // Settings setter/getter
        void setSettingsActivateMultiSessions(bool enabled);
        bool getSettingsActivateMultiSession();

    private:
        // Internal user store
        QMap<QIODevice*, Protocol::Session*> mapSocketsSession;
        QMap<qint32, QMap<QIODevice*, Protocol::Session*>* > mapUsersSessions;
        EleaphRpc* eleaphRPC;

        // Settings
        bool boolSettingsMultiSessionsActive;

    private slots:
        // Post packet handling
        void metaEventUserLoggedInCheck(EleaphRpcDelegate *delegate, EleaphRpcPacket packet, EleaphRpcPacketHandler::EventResult *eventResult);

        // Packet Event handlers
        void handleLogin(EleaphRpcPacket dataPacket);
        void handleLogout(EleaphRpcPacket dataPacket);
        void handleUserInfoSelf(EleaphRpcPacket dataPacket);

        // Low Level handlers
        void handleClientDisconnect(QIODevice *device);

        // Helper functions
        void handleUserChange(Usermanager::SharedSession session, QIODevice *deviceProducerOfChange, Usermanager::UserChangeType changeType);
};

#endif // USERMANAGER_H
