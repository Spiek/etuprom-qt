#ifndef USERMANAGER_H
#define USERMANAGER_H

// Qt (core)
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QIODevice>
#include <QtCore/QPair>
#include <QtCore/QVariant>

// Qt (sql)
#include <QtSql/QSqlQuery>

// own libs
#include "protocol.pb.h"
#include "global.h"

class Usermanager : public QObject
{
    Q_OBJECT
    public:
        // con and decon
        Usermanager(QObject *parent = 0);
        ~Usermanager();

        // protocol helper methods
        void userChanged(QIODevice *device, bool refreshUser = true);
        void userChanged(qint32 userid, bool refreshUser = true);

        // user managment helper methods
        void addUser(QIODevice *device, Protocol::User *user);
        void removeUser(Protocol::User *user);
        void removeUser(QIODevice *device);
        bool isLoggedIn(QIODevice *device);
        bool isLoggedIn(qint32 userID);
        bool refreshUser(Protocol::User *user);
        Protocol::User* getConnectedUser(QIODevice *device);
        Protocol::User* getConnectedUser(qint32 userid);
        QIODevice* getConnectedDevice(qint32 userid);

    private:
        QMap<QIODevice*, Protocol::User*> mapSocketUser;
        QMap<qint32, QPair<QIODevice*, Protocol::User*> > mapIdUser;

    private slots:
        void handle_client_disconnect(QIODevice *device);
};

#endif // USERMANAGER_H
