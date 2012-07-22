#include "databasehelper.h"

bool DatabaseHelper::initialize(QString strDriver, QString strDatabase)
{
    // setup database
    QSqlDatabase db = QSqlDatabase::addDatabase(strDriver);
    db.setDatabaseName(strDatabase);
    return db.open();
}


/*
 * user database ACCESS methods
 */

QSqlQuery DatabaseHelper::getUserById(qint32 userId)
{
    // build query
    QString strQuery = QString("users.id = %1").arg(userId);
    return DatabaseHelper::buildUserQuery(strQuery);
}

QSqlQuery DatabaseHelper::getUserByIdUserNameAndPw(QString strUsername, QString strPassword)
{
    // build query
    QString strQuery = QString("users.username = \"%1\" AND users.password = \"%2\"").arg(strUsername, strPassword);
    return DatabaseHelper::buildUserQuery(strQuery);
}


/*
 * user database UPDATE methods
 */
bool DatabaseHelper::updateUserOnlineState(quint32 userId, bool online)
{
    // build query
    QString strUpdateString = QString("online = %1").arg((int)online);
    return DatabaseHelper::buildUpdateUserQuery(userId, strUpdateString);
}


/*
 * contact-list database ACCESS methods
 */

QSqlQuery DatabaseHelper::getContactsByUserId(qint32 userId)
{
    // build query
    QString strQuery = QString("userlist.user_id = %1 AND users.id = userlist.contact_user_id").arg(userId);
    return DatabaseHelper::buildContactListUsersQuery(strQuery);
}

QSqlQuery DatabaseHelper::getOnlineContactsByUserId(qint32 userId)
{
    // build query
    QString strQuery = QString("userlist.contact_user_id = %1 AND users.online = 1 AND users.id = userlist.user_id").arg(userId);
    return DatabaseHelper::buildContactListUsersQuery(strQuery);
}



/*
 * Low level Database Accesss methods
 */

QSqlQuery DatabaseHelper::buildUserQuery(QString strCondition)
{
    // build user query
    QString strQuery = QString(
                "SELECT "
                "	users.id, "
                "   users.username,"
                "	users.state, "
                "	users.online, "
                "	users.visible "
                "FROM users "
                "WHERE %1;"
    ).arg(strCondition);

    // return constructed query
    return QSqlQuery(strQuery, QSqlDatabase::database());
}

QSqlQuery DatabaseHelper::buildContactListUsersQuery(QString strJoinConditions)
{
    // build query
    QString strQuery = QString(
                "SELECT "
                "	users.id, "
                "	users.username, "
                "	users.state, "
                "	users.online, "
                "	users.visible, "
                "	userlist.group "
                "FROM users "
                "INNER JOIN userlist "
                "ON %1;"
    ).arg(strJoinConditions);

    // exec query and return the result
    return QSqlQuery(strQuery, QSqlDatabase::database());
}

bool DatabaseHelper::buildUpdateUserQuery(quint32 userId, QString strFields)
{
    // build query
    QString strQuery = QString(
                "UPDATE "
                "	users SET %1 "
                "WHERE id = %2;"
    ).arg(strFields).arg(userId);

    // exec query and catch return value and return it
    return QSqlQuery(strQuery, QSqlDatabase::database()).exec();
}

