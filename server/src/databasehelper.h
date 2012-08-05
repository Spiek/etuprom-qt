#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H

// Qt (core)
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariant>

// Qt (sql)
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

// protobuf libs
#include "protocol.pb.h"

class DatabaseHelper
{
    public:
        static bool initialize(QString strDriver, QString strDatabase);

        // user database access methods
        static QSqlQuery getUserById(qint32 userId);
        static QSqlQuery getUserByIdUserNameAndPw(QString strUsername, QString strPassword);

        // user database update methods
        static bool updateUserOnlineState(quint32 userId, bool online);

        // contact-list database access methods
        static QSqlQuery getContactsByUserId(qint32 userId);
        static QSqlQuery getOnlineContactsByUserId(qint32 intId);

        // messages database access methods
        static bool createNewUserMessage(qint32 userSenderId, qint32 userReceiverId, QString strMessage);

    private:
        static QSqlQuery buildUserQuery(QString strCondition);
        static QSqlQuery buildContactListUsersQuery(QString strJoinConditions);

        static bool buildUpdateUserQuery(quint32 userId, QString strFields);
};

#endif // DATABASEHELPER_H
