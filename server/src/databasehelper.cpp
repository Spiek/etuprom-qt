#include "databasehelper.h"

//
// Con(s) / Decon
//

DatabaseHelper::DatabaseHelper(QString strDatabaseInitialzer)
{
    // save database initializer
    this->strDatabaseInitialzer = strDatabaseInitialzer;

    // read table definations
    this->queryBuilder = new QueryBuilder(strDatabaseInitialzer);
    this->queryBuilder->updateTableSchemaCache();
}


//
// user database ACCESS methods
//

bool DatabaseHelper::getUserById(qint32 userId, Protocol::User* user)
{
    // simplefy table name
    QString strTableName = "user";

    // query: construct expressions
    this->queryBuilder->clear(QueryBuilder::SELECT);
    if(!this->queryBuilder->addSelectExpressionTable(strTableName)) {
        qWarning("[%s][%d] - Can not add table Expression for table \"%s\"", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }
    // query: add where conditions to query
    this->queryBuilder->addWhereCondition(strTableName, "id", QString::number(userId), true);

    // query: build
    QString strQuery = this->queryBuilder->buildQuery();

    // exec query, if no data was found, return false
    QSqlQuery query = this->buildQuery(strQuery);
    if(!query.next()) {
        return false;
    }

    // convert result to protobuf message
    if(!DatabaseHelper::queryToProtobufMessage(query, user)) {
        qWarning("[%s][%d] - Cannot convert Query for %s To Protobuf Message", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }

    // all okay, return true
    return true;
}

bool DatabaseHelper::getUserByIdUserNameAndPw(QString strUsername, QString strPassword, Protocol::User* user)
{
    // simplefy table name
    QString strTableName = "user";

    // query: construct expressions
    this->queryBuilder->clear(QueryBuilder::SELECT);
    if(!this->queryBuilder->addSelectExpressionTable(strTableName)) {
        qWarning("[%s][%d] - Can not add table Expression for table \"%s\"", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }
    // query: add where conditions to query
    this->queryBuilder->addWhereCondition(strTableName, "username", strUsername, false, "AND");
    this->queryBuilder->addWhereCondition(strTableName, "password", strPassword);

    // query: build
    QString strQuery = this->queryBuilder->buildQuery();

    // exec query, if no data was found, return false
    QSqlQuery query = this->buildQuery(strQuery);
    if(!query.next()) {
        return false;
    }

    // convert result to protobuf message
    if(!DatabaseHelper::queryToProtobufMessage(query, user)) {
        qWarning("[%s][%d] - Cannot convert Query for %s To Protobuf Message", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }

    // all okay, return true
    return true;
}


//
// contact-list database access methods
//

bool DatabaseHelper::getContactsByUserId(qint32 userId, Protocol::ContactList* contactList)
{
    // simplefy table name
    QString strTableName = "user";

    // query: construct expressions
    this->queryBuilder->clear(QueryBuilder::SELECT);
    if(!this->queryBuilder->addSelectExpressionTable(strTableName)) {
        qWarning("[%s][%d] - Can not add table Expression for table \"%s\"", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }
    this->queryBuilder->addSelectExpression("userlist", "group", "contact.group");

    // query: add where conditions to query
    this->queryBuilder->addWhereCondition("userlist", "user_id", QString::number(userId), true);

    // query: build
    QString strQuery = this->queryBuilder->buildQuery();

    // exec query, if no data was found, return false
    QSqlQuery query = this->buildQuery(strQuery);
    if(!query.next()) {
        return false;
    }

    // convert result to protobuf message
    if(!DatabaseHelper::queryToProtobufMessage(query, contactList)) {
        qWarning("[%s][%d] - Cannot convert Query for %s To Protobuf Message", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }

    // all okay, return true
    return true;
}

bool DatabaseHelper::getOnlineContactsByUserId(qint32 userId, Protocol::Users* users)
{
    // simplefy table name
    QString strTableName = "user";

    // query: construct expressions
    this->queryBuilder->clear(QueryBuilder::SELECT);
    if(!this->queryBuilder->addSelectExpressionTable(strTableName)) {
        qWarning("[%s][%d] - Can not add table Expression for table \"%s\"", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }
    this->queryBuilder->addSelectExpression("userlist", "group", "contact.group");

    // query: add where conditions to query
    this->queryBuilder->addWhereCondition("userlist", "user_id", QString::number(userId), true, "AND");
    this->queryBuilder->addWhereCondition(strTableName, "online", QString::number(1), true);

    // query: build
    QString strQuery = this->queryBuilder->buildQuery();

    // exec query, if no data was found, return false
    QSqlQuery query = this->buildQuery(strQuery);
    if(!query.next()) {
        return false;
    }

    // convert result to protobuf message
    if(!DatabaseHelper::queryToProtobufMessage(query, users)) {
        qWarning("[%s][%d] - Cannot convert Query for %s To Protobuf Message", __PRETTY_FUNCTION__ , __LINE__, qPrintable(strTableName));
        return false;
    }

    // all okay, return true
    return true;
}


//
// update database access methods
//

bool DatabaseHelper::updateUserById(Protocol::User *user)
{
    // simplefy table name
    QString strTableName = "user";

    // query: construct expressions
    this->queryBuilder->clear(QueryBuilder::UPDATE);

    // query: set database table
    this->queryBuilder->setUpdateTable(strTableName);

    // query: build where condition on id
    this->queryBuilder->addWhereCondition(strTableName, "id", QString::number(user->id()), true);

    // query: build update fields for entire protobuf message user
    this->createUpdateQueryForProtbuf(user);

    // query: build
    QString strQuery = this->queryBuilder->buildQuery();

    // exec query, if no data was found, return false
    QSqlQuery query = this->buildQuery(strQuery);

    // return query executing result
    return query.exec();
}

bool DatabaseHelper::updateUserOnlineStateById(Protocol::User *user, bool onlineState)
{
    // simplefy table name
    QString strTableName = "user";

    // query: construct expressions
    this->queryBuilder->clear(QueryBuilder::UPDATE);

    // query: set database table
    this->queryBuilder->setUpdateTable(strTableName);

    // query: build where condition on id
    this->queryBuilder->addWhereCondition(strTableName, "id", QString::number(user->id()), true);

    // query: set update field online
    this->queryBuilder->addUpdateField("online", onlineState ? "1" : "0", true);

    // query: build
    QString strQuery = this->queryBuilder->buildQuery();

    // exec query, if no data was found, return false
    QSqlQuery query = this->buildQuery(strQuery);

    // return query executing result
    return query.exec();
}



//
// protobuf helpers
//

bool DatabaseHelper::queryToProtobufMessage(QString query, google::protobuf::Message *message)
{
    return DatabaseHelper::queryToProtobufMessage(DatabaseHelper::buildQuery(query), message);
}

bool DatabaseHelper::queryToProtobufMessage(QSqlQuery query, google::protobuf::Message *message, bool firstLayer)
{
    // simplefy some protobuf classes
    const google::protobuf::Descriptor *descriptor = message->GetDescriptor();
    const google::protobuf::Reflection *reflector = message->GetReflection();
    QString strTableName = QString::fromStdString(descriptor->name());

    bool repeated = false;
    do {
        QSqlRecord record = query.record();

        for(int i = 0;i < descriptor->field_count(); i++) {
            // simplefy some protobuf helpers
            const google::protobuf::FieldDescriptor* fieldDescriptor = descriptor->field(i);
            google::protobuf::FieldDescriptor::CppType typeCpp = google::protobuf::FieldDescriptor::TypeToCppType(fieldDescriptor->type());
            QString strProtoFieldName = QString("%1.%2").arg(strTableName, QString::fromStdString(fieldDescriptor->name()));

            // handle protobuf message fieldtype
            if(typeCpp == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
                google::protobuf::Message* subMessage;
                if(fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED) {
                    subMessage = reflector->AddMessage(message, fieldDescriptor);
                } else {
                    subMessage = reflector->MutableMessage(message, fieldDescriptor);
                }
                this->queryToProtobufMessage(query, subMessage, false);
            }

            // otherwise check if fieldname in sql query exist, and exit if not
            else if(record.indexOf(strProtoFieldName) == -1) {
                qWarning("Error protobuf field \"%s\" couldn't be found in SQL Query result!", qPrintable(strProtoFieldName));
            }

            // write sql value to protobuf field
            else if(!DatabaseHelper::setFieldValueByFieldDescriptor(message, fieldDescriptor, query.value(record.indexOf(strProtoFieldName)))) {
                 qWarning("Error in database/protobuf data schema! [DB FIELD: \"%s\" with value (string)\"%s\"couldn't be converted to PROTOBUF fieldtype \"%d\"!]", qPrintable(strProtoFieldName), qPrintable(query.value(record.indexOf(strProtoFieldName)).toString()), (int)typeCpp);
            }

            // set repeated flag
            if(fieldDescriptor->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED) {
                repeated = true;
            }
        }
    } while(firstLayer && repeated && query.next());

    // success
    return true;
}

QVariant DatabaseHelper::getFieldValueByFieldDescriptor(google::protobuf::Message *message, const google::protobuf::FieldDescriptor *fieldDescriptor)
{
    // exit here if something is wrong with the given params
    if(!message || !fieldDescriptor) {
        return QVariant::Invalid;
    }

    // build QVariant Value
    QVariant variant;
    const google::protobuf::Reflection *reflector = message->GetReflection();
    google::protobuf::FieldDescriptor::CppType typeCpp = google::protobuf::FieldDescriptor::TypeToCppType(fieldDescriptor->type());
    switch(typeCpp) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32 :
            variant.setValue(reflector->GetInt32(*message, fieldDescriptor));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64 :
            variant.setValue(reflector->GetInt64(*message, fieldDescriptor));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32 :
            variant.setValue(reflector->GetUInt32(*message, fieldDescriptor));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64 :
            variant.setValue(reflector->GetUInt64(*message, fieldDescriptor));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE :
            variant.setValue(reflector->GetDouble(*message, fieldDescriptor));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT :
            variant.setValue(reflector->GetFloat(*message, fieldDescriptor));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL :
            variant.setValue(reflector->GetBool(*message, fieldDescriptor) ? '1' : '0');
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM :
            variant.setValue(reflector->GetEnum(*message, fieldDescriptor)->index());
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING :
            variant.setValue(QString::fromStdString(reflector->GetString(*message, fieldDescriptor)));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE :

        break;
    }

    return variant;
}

bool DatabaseHelper::setFieldValueByFieldDescriptor(google::protobuf::Message *message, const google::protobuf::FieldDescriptor *fieldDescriptor, QVariant value)
{
    // exit here if something is wrong with the given params
    if(!message || !fieldDescriptor || value == QVariant::Invalid) {
        return false;
    }

    // build QVariant Value
    const google::protobuf::Reflection *reflector = message->GetReflection();
    google::protobuf::FieldDescriptor::CppType typeCpp = google::protobuf::FieldDescriptor::TypeToCppType(fieldDescriptor->type());
    bool okay = true;
    switch(typeCpp) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32 :
            reflector->SetInt32(message, fieldDescriptor, value.toInt(&okay));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64 :
            reflector->SetInt64(message, fieldDescriptor, value.toLongLong(&okay));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32 :
            reflector->SetUInt32(message, fieldDescriptor, value.toUInt(&okay));;
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64 :
            reflector->SetUInt64(message, fieldDescriptor, value.toULongLong(&okay));
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE :
            reflector->SetDouble(message, fieldDescriptor, value.toDouble(&okay));;
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT :
            reflector->SetFloat(message, fieldDescriptor, value.toFloat(&okay));;
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL :
            reflector->SetBool(message, fieldDescriptor, value.toBool());
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING :
            reflector->SetString(message, fieldDescriptor, value.toString().toStdString());
        break;
        default :
            return false;
        break;
    }

    return okay;
}


//
// protobuf query helpers
//

void DatabaseHelper::createUpdateQueryForProtbuf(google::protobuf::Message *message)
{
    // simplefy some protobuf classes
    const google::protobuf::Descriptor *descriptor = message->GetDescriptor();

    for(int i = 0;i < descriptor->field_count(); i++) {
        // simplefy some protobuf helpers
        const google::protobuf::FieldDescriptor* fieldDescriptor = descriptor->field(i);
        google::protobuf::FieldDescriptor::CppType typeCpp = google::protobuf::FieldDescriptor::TypeToCppType(fieldDescriptor->type());

        // skip all not acceptable fields
        if(typeCpp == google::protobuf::FieldDescriptor::CPPTYPE_ENUM || typeCpp == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
            continue;
        }

        // get value and build update query
        QVariant varValue = this->getFieldValueByFieldDescriptor(message, fieldDescriptor);
        this->queryBuilder->addUpdateField(QString::fromStdString(fieldDescriptor->name()).toLower(), typeCpp == google::protobuf::FieldDescriptor::CPPTYPE_STRING ? '"' + varValue.toString() + '"' : varValue.toString());
    }
}





/*
bool DatabaseHelper::insertByProtocolBufferMessage(google::protobuf::Message *message, bool skipId)
{
    // simplefy some protobuf classes
    const google::protobuf::Descriptor *descriptor = message->GetDescriptor();
    QString strMessageName = QString::fromStdString(descriptor->name()).toLower();

    // build query construct
    QString strQuery = QString("INSERT INTO `%1`(%2) VALUES(%3);").arg(strMessageName);
    QString strQueryColumnDefinition;
    QString strQueryValueDefinition;

    // build query
    foreach(QString strFieldName, DatabaseHelper::mapTableSchema.value(strMessageName)) {
        // skip id field
        if(skipId && strFieldName.toLower() == "id") {
            continue;
        }

        // get some protobuf helper objects
        const google::protobuf::FieldDescriptor* fieldDescriptor = descriptor->FindFieldByName(strFieldName.toStdString());

        // skip field if field doesn't exist in protbuf message
        if(!fieldDescriptor) {
            continue;
        }

        // add column defination
        strQueryColumnDefinition += strFieldName + ',';

        // add value for column
        // handle string values
        QVariant varValue = DatabaseHelper::getFieldValueByFieldDescriptor(message, fieldDescriptor);
        if(google::protobuf::FieldDescriptor::TypeToCppType(fieldDescriptor->type()) == 9) {
            strQueryValueDefinition += '\'' + varValue.toString() + '\'';
        }

        // handle numeric values
        else {
            strQueryValueDefinition += varValue.toString();
        }

        // finialize value
        strQueryValueDefinition += ',';
    }

    // exit here if we couldn't extract at least one column for the insert query
    if(strQueryColumnDefinition.isEmpty()) {
        return false;
    }

    // ... otherwise we finializing the query
    strQueryColumnDefinition = strQueryColumnDefinition.left(strQueryColumnDefinition.length() - 1);
    strQueryValueDefinition = strQueryValueDefinition.left(strQueryValueDefinition.length() - 1);
    strQuery = strQuery.arg(strQueryColumnDefinition).arg(strQueryValueDefinition);

    // now we execute the query
    return QSqlQuery(strQuery).exec();
}

bool DatabaseHelper::updateByProtocolBufferMessageById(google::protobuf::Message *message)
{
    // simplefy some protobuf classes
    const google::protobuf::Descriptor *descriptor = message->GetDescriptor();
    QString strMessageName = QString::fromStdString(descriptor->name()).toLower();

    // build query construct
    QString strQuery = QString("UPDATE `%1` SET %4 WHERE `%1`.`%2` = %3;").arg(strMessageName);
    QString strQuerySetDefinition;

    // get and set to query id
    QString strIdField = "id";
    qint64 id = 0;
    const google::protobuf::FieldDescriptor* fieldDescriptorId = descriptor->FindFieldByLowercaseName(strIdField.toStdString());
    if(!fieldDescriptorId) {
        return false;
    } else {
        id = DatabaseHelper::getFieldValueByFieldDescriptor(message, fieldDescriptorId).toLongLong();
    }
    strQuery = strQuery.arg(strIdField).arg(id);

    // build query
    foreach(QString strFieldName, DatabaseHelper::mapTableSchema.value(strMessageName)) {
        // get some protobuf helper objects
        const google::protobuf::FieldDescriptor* fieldDescriptor = descriptor->FindFieldByName(strFieldName.toStdString());

        // skip field if field doesn't exist in protbuf message
        if(!fieldDescriptor) {
            continue;
        }

        // add value for column
        strQuerySetDefinition += strFieldName + "=";

        // handle string values
        QVariant varValue = DatabaseHelper::getFieldValueByFieldDescriptor(message, fieldDescriptor);
        if(google::protobuf::FieldDescriptor::TypeToCppType(fieldDescriptor->type()) == 9) {
            strQuerySetDefinition += '\'' + varValue.toString() + '\'';
        }

        // handle numeric values
        else {
            strQuerySetDefinition += varValue.toString();
        }

        // finialize value
        strQuerySetDefinition += ',';
    }

    // exit here if we couldn't extract at least one column for the insert query
    if(strQuerySetDefinition.isEmpty()) {
        return false;
    }

    // ... otherwise we finializing the query
    strQuerySetDefinition = strQuerySetDefinition.left(strQuerySetDefinition.length() - 1);
    strQuery = strQuery.arg(strQuerySetDefinition);

    // now we execute the query
    return QSqlQuery(strQuery).exec();
}
*/

//
// global helpers
//

QSqlQuery DatabaseHelper::buildQuery(QString strQuery)
{
    return QSqlQuery(strQuery, QSqlDatabase::database(DatabaseHelper::strDatabaseInitialzer));
}
