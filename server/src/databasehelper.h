/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H

// Qt (core)
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QPair>

// Qt (sql)
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>

// protbuf
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

// own protobuf defination
#include "protocol.pb.h"
#include "querybuilder.h"

class DatabaseHelper
{
    public:
        // initializer
        DatabaseHelper(QString strDatabaseInitialzer);

        // user database ACCESS methods
        bool getUserById(qint32 userId, Protocol::User* user);
        bool getUserByIdUserNameAndPw(QString strUsername, QString strPassword, Protocol::User* user);

        // contact-list database access methods
        bool getContactsByUserId(qint32 userId, Protocol::ContactList *contactList);
        bool getOnlineContactsByUserId(qint32 intId, Protocol::Users *users);
        bool getAllOnlineContactsByUserId(qint32 intId, Protocol::Users *users);

        // update database access methods
        bool updateUserById(Protocol::User *user);
        bool updateUserOnlineStateById(Protocol::User *user, bool onlineState);

    private:
        // helper data
        QueryBuilder *queryBuilder;
        QString strDatabaseInitialzer;

        // protobuf helpers
        bool queryToProtobufMessage(QString query, google::protobuf::Message* message);
        bool queryToProtobufMessage(QSqlQuery query, google::protobuf::Message* message, bool firstLayer = true); // FIXME: make private recurive method, so that bool firstLayer = true cannot set extern
        QVariant getFieldValueByFieldDescriptor(google::protobuf::Message *message, const google::protobuf::FieldDescriptor *fieldDescriptor);
        bool setFieldValueByFieldDescriptor(google::protobuf::Message *message, const google::protobuf::FieldDescriptor *fieldDescriptor, QVariant value);

        // protobuf query helpers
        QueryBuilder *createUpdateQueryForProtbuf(QueryBuilder *queryBuilder, google::protobuf::Message *message);
        /*bool insertByProtocolBufferMessage(google::protobuf::Message *message, bool skipId = true);
        bool updateByProtocolBufferMessageById(google::protobuf::Message *message); SOME BUGS TO FIX UNTIL WE CAN USE IT!*/

        // global helpers
        QSqlQuery buildQuery(QString strQuery);
};

#endif // DATABASEHELPER_H
