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
#include "global.h"

// protobuf libs
#include "protocol.pb.h"

class Usermanager : public QObject
{
    Q_OBJECT
    public:
        // protocol helper methods
        void userChanged(QIODevice *device);
        void userChanged(qint32 userid);

        // database user helper methods
        Protocol::User* setUserfromQuery(QSqlQuery *query,  Protocol::User* user = 0);

        // user managment helper methods
        void addUser(QIODevice *device, Protocol::User *user);
        void removeUser(Protocol::User *user);
        void removeUser(QIODevice *device);
        Protocol::User* refreshUser(Protocol::User *user);
        Protocol::User* getConnectedUser(QIODevice *device);
        Protocol::User* getConnectedUser(qint32 userid);
        QIODevice* getConnectedDevice(qint32 userid);

    private:
        QMap<QIODevice*, Protocol::User*> mapSocketUser;
        QMap<qint32, QPair<QIODevice*, Protocol::User*> > mapIdUser;

    // Singelton members
    private:
        static Usermanager *userManager;

    protected:
        Usermanager(QObject *parent = 0);
        ~Usermanager();

    public:
        static void create(QObject *parent = 0);
        static Usermanager* getInstance();
};

#endif // USERMANAGER_H
