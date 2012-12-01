#include "querybuilder.h"

QueryBuilder::QueryBuilder(QString strDatabaseIntefier)
{
    // save database intentifier
    this->strDatabaseIntefier = strDatabaseIntefier;
    this->intLimit = -1;
}

//
// Query functions
//

void QueryBuilder::updateTableSchemaCache()
{
    // select all tables and loop one by one
    QSqlDatabase database = QSqlDatabase::database(this->strDatabaseIntefier);

    // loop all tables
    this->mapTableSchema.clear();
    foreach(QString strTable, database.tables()) {
        // skip database_associations
        if(strTable == "database_associations") {
            continue;
        }

        // get table record
        QSqlRecord record = database.record(strTable);

        // loop all fields
        QMap<QString, QVariant::Type> mapColumns;
        for(int i = 0; i < record.count(); i++) {
            // extract field informations
            QSqlField field = record.field(i);
            QString strFieldName = field.name();
            QVariant::Type type = field.type();
            mapColumns.insert(strFieldName, type);
        }

        // add table defination
        this->mapTableSchema.insert(strTable, mapColumns);
    }

    // read database_associations
    QSqlQuery queryAssociations("SELECT * FROM database_associations", database);
    while(queryAssociations.next()) {
        // fill SELECTAssociation struct
        QueryBuilder::SELECTAssociation selectAssociation;
        selectAssociation.strTable = queryAssociations.value(1).toString();
        selectAssociation.strColumn = queryAssociations.value(2).toString();
        selectAssociation.strJoinTable = queryAssociations.value(3).toString();
        selectAssociation.strJoinColumn = queryAssociations.value(4).toString();
        selectAssociation.enumRelation = queryAssociations.value(5).toString() == "hasOne" ? QueryBuilder::SELECTAssociation::hasOne : (queryAssociations.value(4).toString() == "hasMany") ? QueryBuilder::SELECTAssociation::hasMany : QueryBuilder::SELECTAssociation::belongsTo;

        // save Association
        this->mapSelectAssociations.insert(selectAssociation.strTable, selectAssociation);
    }
}

void QueryBuilder::createNewQuery(QueryBuilder::QueryType queryType)
{
    // clear pref. constructed query
    this->clear();

    // save current query type
    this->currentQueryType = queryType;
}

void QueryBuilder::clear(QueryBuilder::QueryType queryType)
{
    // set new query type
    this->currentQueryType = queryType;

    // Cleanup
    // Global
    qDeleteAll(this->mapWhereConditions);
    this->mapWhereConditions.clear();

    // Select
    this->strFromTable.clear();
    this->mapJoins.clear();
    this->mapSelectExpressions.clear();
    this->lstGroupBy.clear();
    qDeleteAll(this->mapHavingConditions);
    this->mapHavingConditions.clear();
    this->lstOrderBy.clear();
    this->intLimit = -1;

    // Update
    this->strUpdateTable.clear();
    this->lstUpdateFields.clear();

    // Insert
    this->strInsertTable.clear();
    this->mapInsertFields.clear();
    this->strDeleteTable.clear();


}

QString QueryBuilder::buildQuery()
{
    // build query
    QString strQuery;
    switch(this->currentQueryType) {
        case QueryBuilder::SELECT :
            this->constructQuery_Select(&strQuery);
        break;
        case QueryBuilder::UPDATE :
            this->constructQuery_Update(&strQuery);
        break;
        case QueryBuilder::INSERT :
            this->constructQuery_Insert(&strQuery);
        break;
        case QueryBuilder::DELETE :
            this->constructQuery_Delete(&strQuery);
        break;
    }

    // build where condition
    if(this->currentQueryType != QueryBuilder::INSERT && !this->mapWhereConditions.isEmpty()) {
        strQuery += " WHERE ";
        this->constructQueryPart_Condition(&strQuery, &this->mapWhereConditions);
    }

    // SELECT: add group by
    if(this->currentQueryType == QueryBuilder::SELECT && !this->lstGroupBy.isEmpty()) {
        this->constructQuery_Select_GroupBy(&strQuery);
    }

    // SELECT: build having condition (if we have at least one column we group by)
    if(this->currentQueryType == QueryBuilder::SELECT && !this->lstGroupBy.isEmpty() && !this->mapHavingConditions.isEmpty()) {
        strQuery += " HAVING ";
        this->constructQueryPart_Condition(&strQuery, &this->mapHavingConditions);
    }

    // SELECT: Order By
    if(this->currentQueryType != QueryBuilder::INSERT && !this->lstOrderBy.isEmpty()) {
        this->constructQuery_Select_OrderBy(&strQuery);
    }

    // SELECT: add limit
    if(this->currentQueryType != QueryBuilder::INSERT && this->intLimit != -1) {
        strQuery.append(" LIMIT " + QString::number(this->intLimit));
    }

    return strQuery;
}


//
// Universal Query functions
//

void QueryBuilder::addWhereCondition(QString strTable, QString strField, QString strValue, bool isNumeric, QString strOpperator, int level)
{
    // create condition
    if(!this->mapWhereConditions.contains(level)) {
        this->mapWhereConditions.insert(level, new QList<QueryBuilder::Condition>());
    }
    QueryBuilder::Condition condition;
    condition.strTable = strTable;
    condition.strColumn = strField;
    condition.strValue = strValue;
    condition.strOpperator = strOpperator;
    condition.isNumeric = isNumeric;
    this->mapWhereConditions.value(level)->append(condition);
}


//
// Select Query functions
//

bool QueryBuilder::addSelectExpressionTable(QString strTable, QString strTableAlias, QueryBuilder::JoinType joinTypeIfNeccessary)
{
    // exist if table doesn't exist
    if(!this->mapTableSchema.contains(strTable)) {
        return false;
    }

    // ...otherwise add table columns
    foreach(QString strColumnName, this->mapTableSchema.value(strTable).keys()) {
        QString strExpression = QString("%1.%2").arg(strTable, strColumnName);
        QString strAlias = QString("%1.%2").arg(strTableAlias.isEmpty() ? strTable : strTableAlias, strColumnName);
        this->mapSelectExpressions.insert(strExpression, strAlias);
    }

    // set from table if not allready happened
    if(this->strFromTable.isEmpty()) {
        this->strFromTable = strTable;
    }

    // if from table = added table, don't do a join
    else if(this->strFromTable == strTable) {
        return true;
    }

    // add a join if table is not the from table
    else if(this->mapSelectAssociations.contains(strTable)) {
        this->addSelectJoin(strTable, joinTypeIfNeccessary);
    }

    else {
        return false;
    }

    // otherwise exit with error
    return true;
}

bool QueryBuilder::addSelectExpression(QString strTable, QString strColumn, QString strAlias, QueryBuilder::JoinType joinTypeIfNeccessary)
{
    // exit if table and column doesn't exist in database
    if(!this->mapTableSchema.contains(strTable) || !this->mapTableSchema.value(strTable).contains(strColumn)) {
        return false;
    }

    // build column expression
    QString strExpression = QString("%1.%2").arg(strTable, strColumn);
    QString strColumnAlias;
    if(strAlias.isEmpty()) {
        strColumnAlias = strExpression;
    } else {
        strColumnAlias = strAlias;
    }
    this->mapSelectExpressions.insert(strExpression, strColumnAlias);

    // set from table if not allready happened
    if(this->strFromTable.isEmpty()) {
        this->strFromTable = strTable;
    }

    // if from table = added table, don't do a join
    else if(this->strFromTable == strTable) {
        return true;
    }

    // add a join if table is not the from table
    else if(this->mapSelectAssociations.contains(strTable)) {
        this->addSelectJoin(strTable, joinTypeIfNeccessary);
    }

    else {
        return false;
    }

    // otherwise exit with error
    return true;
}

bool QueryBuilder::addSelectJoin(QString strTable, QueryBuilder::JoinType type)
{
    // if join is not defined in assioaction table, exit here
    if(!this->mapSelectAssociations.contains(strTable)) {
        return false;
    }

    // otherwise save the join
    this->mapJoins.insert(strTable, type);
    return true;
}

bool QueryBuilder::addSelectGroupBy(QString strTable, QString strColumn)
{
    // if we don't select the group by field, return false
    QString strGroupBy = QString("%1.%2").arg(strTable, strColumn);
    if(!this->mapSelectExpressions.contains(QString("%1.%2").arg(strTable, strColumn))) {
        return false;
    }

    // otherwise add the group by field to list
    this->lstGroupBy.append(strGroupBy);
    return true;
}

void QueryBuilder::addSelectHavingCondition(QString strTable, QString strField, QString strValue, QString strOpperator, int level, bool isNumeric)
{
    // create condition
    if(!this->mapHavingConditions.contains(level)) {
        this->mapHavingConditions.insert(level, new QList<QueryBuilder::Condition>());
    }
    QueryBuilder::Condition condition;
    condition.strTable = strTable;
    condition.strColumn = strField;
    condition.strValue = strValue;
    condition.strOpperator = strOpperator;
    condition.isNumeric = isNumeric;
    this->mapHavingConditions.value(level)->append(condition);
}

bool QueryBuilder::addOrderBy(QString strTable, QString strColumn, QueryBuilder::OrderType orderType)
{
    // if we don't select the group by field, return false
    QString strOrderBy = QString("%1.%2").arg(strTable, strColumn);
    if(!this->mapSelectExpressions.contains(strOrderBy)) {
        return false;
    }

    // add order by entry
    this->lstOrderBy.append(QString(" %1 %2").arg(strOrderBy, orderType == QueryBuilder::ASC ? "ASC" : "DESC"));
    return true;
}

void QueryBuilder::setLimit(int limit)
{
    this->intLimit = limit;
}


//
// Update Query functions
//

bool QueryBuilder::setUpdateTable(QString strTable)
{
    // exist if table doesn't exist
    if(!this->mapTableSchema.contains(strTable)) {
        return false;
    }

    // set update table
    this->strUpdateTable = strTable;
    return true;
}

bool QueryBuilder::addUpdateField(QString strField, QString strValue, bool isNumeric)
{
    // exist if table and column doesn't exist
    if(!this->mapTableSchema.contains(this->strUpdateTable) || !this->mapTableSchema.value(this->strUpdateTable).contains(strField)) {
        return false;
    }

    // append field to list
    this->lstUpdateFields.append(QString("%1 = %2").arg(strField, isNumeric ? strValue : '"' + strValue + '"'));
    return true;
}


//
// Insert Query functions
//

bool QueryBuilder::setInsertTable(QString strTable)
{
    // exist if table doesn't exist
    if(!this->mapTableSchema.contains(strTable)) {
        return false;
    }

    // otherwise set insert table
    this->strInsertTable = strTable;
    return true;
}

bool QueryBuilder::setInsertField(QString strFieldName, QString strValue, bool isNumeric)
{
    // exist if table and column doesn't exist
    if(!this->mapTableSchema.contains(this->strInsertTable) || !this->mapTableSchema.value(this->strInsertTable).contains(strFieldName)) {
        return false;
    }

    // otherwise add the field
    this->mapInsertFields.insert(strFieldName, isNumeric ? strValue : '"'  + strValue + '"');
    return true;
}

//
// Delete Query functions
//

bool QueryBuilder::setDeleteTable(QString strTable)
{
    // exist if table doesn't exist
    if(!this->mapTableSchema.contains(strTable)) {
        return false;
    }

    // otherwise set insert table
    this->strDeleteTable = strTable;
    return true;
}

//
// Private Construction methods
//

void QueryBuilder::constructQuery_Select(QString *strQuery)
{
    *strQuery = QString("SELECT %1 FROM %2");

    // build select expression
    QString strJoinedSelectExpression;
    foreach(QString strRealColumn, this->mapSelectExpressions.keys()) {
        strJoinedSelectExpression += QString("%1 as '%2',").arg(strRealColumn, this->mapSelectExpressions.value(strRealColumn));
    }
    *strQuery = strQuery->arg(strJoinedSelectExpression.left(strJoinedSelectExpression.length() -1), this->strFromTable);

    // build joins
    QString strJoin;
    foreach(QString strJoinTable, this->mapJoins.keys()) {
        // create string join
        QString strJoinType = this->mapJoins.value(strJoinTable) == QueryBuilder::INNER ? "INNER" : (this->mapJoins.value(strJoinTable) == QueryBuilder::LEFT ? "LEFT" : "RIGHT");

        // all other tables will be joined
        foreach(QueryBuilder::SELECTAssociation selectAssociation, this->mapSelectAssociations.values(strJoinTable)) {
            strJoin += QString(" %5 JOIN %1 ON %1.%2 = %3.%4 ").arg(selectAssociation.strTable, selectAssociation.strColumn, selectAssociation.strJoinTable, selectAssociation.strJoinColumn, strJoinType);
        }
    }
    *strQuery += strJoin;
}

void QueryBuilder::constructQuery_Select_GroupBy(QString *strQuery)
{
    // add group by fields
    *strQuery += " GROUP BY ";
    foreach(QString strGroupBy, this->lstGroupBy) {
        *strQuery += strGroupBy;
        if(strGroupBy != this->lstGroupBy.last()) {
            *strQuery += ",";
        }
    }
}

void QueryBuilder::constructQuery_Select_OrderBy(QString *strQuery)
{
    // add group by fields
    *strQuery += " ORDER BY ";
    foreach(QString strOrderBy, this->lstOrderBy) {
        *strQuery += strOrderBy;
        if(strOrderBy != this->lstOrderBy.last()) {
            *strQuery += ",";
        }
    }
}

void QueryBuilder::constructQueryPart_Condition(QString* strQuery, QMap<int, QList<QueryBuilder::Condition>*>* mapConditions)
{
    // build conditions
    QString strWhere = "";
    QString strSubWhere = "";
    QString strLastOperator = "";
    bool operatorPossible = false;
    foreach(int level, mapConditions->keys()) {
        strSubWhere += "(";
        strWhere += QString("%1(").arg(strLastOperator);
        operatorPossible = false;
        foreach(QueryBuilder::Condition condition, *mapConditions->value(level)) {
            if(operatorPossible) {
                strWhere += QString("%1").arg(" " + strLastOperator + " ");
            }
            strWhere += QString("%1.%2 = %3").arg(condition.strTable, condition.strColumn, condition.isNumeric ? condition.strValue : '"' + condition.strValue + '"');
            operatorPossible = true;
            strLastOperator = condition.strOpperator;
        }
        strWhere += "))";
        operatorPossible = true;
    }
    *strQuery += strWhere.prepend(strSubWhere);
}

void QueryBuilder::constructQuery_Update(QString *strQuery)
{
    *strQuery += QString("UPDATE %1 SET ").arg(this->strUpdateTable);
    foreach(QString strUpdateField, this->lstUpdateFields) {
        *strQuery += strUpdateField;
        if(strUpdateField != this->lstUpdateFields.last()) {
            *strQuery += ",";
        }
    }
}

void QueryBuilder::constructQuery_Insert(QString *strQuery)
{
    *strQuery += QString("INSERT INTO %1(%2)VALUES(%3)").arg(this->strInsertTable);
    QString strInsertFields = "";
    QString strInsertFieldValues = "";
    foreach(QString strFieldname, this->mapInsertFields.keys()) {
        strInsertFields += strFieldname + ',';
        strInsertFieldValues += this->mapInsertFields.value(strFieldname) + ',';
    }
    *strQuery = strQuery->arg(strInsertFields.left(strInsertFields.length() -1), strInsertFieldValues.left(strInsertFieldValues.length() - 1));
}

void QueryBuilder::constructQuery_Delete(QString *strQuery)
{
    *strQuery += QString("DELETE FROM %1").arg(this->strDeleteTable);
}
