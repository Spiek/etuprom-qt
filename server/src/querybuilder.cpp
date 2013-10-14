/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#include "querybuilder.h"

// init static values
QMap<QString, QMap<QString, QVariant::Type> > QueryBuilder::mapTableSchema;
QString QueryBuilder::strDatabaseIntefier;
QMap<QString, QueryBuilder::SelectAssociation> QueryBuilder::mapSelectAssociations;
bool QueryBuilder::boolIsInitalized = false;

//
// static initializer
//

bool QueryBuilder::initialize(QString strDatabaseIntefier)
{
    // exit if the Querybuilder was allready successfull initalized
    if(QueryBuilder::boolIsInitalized) {
        return true;
    }

    // initialize Database
    QueryBuilder::strDatabaseIntefier = strDatabaseIntefier;
    QSqlDatabase database = QSqlDatabase::database(strDatabaseIntefier);

    // exit here if database couldn't be opened
    if(!database.isOpen() && !database.open()) {
        return false;
    }

    // loop all tables
    QueryBuilder::mapTableSchema.clear();
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
        QueryBuilder::mapTableSchema.insert(strTable, mapColumns);
    }

    // read database_associations
    QSqlQuery queryAssociations("SELECT * FROM database_associations", database);
    while(queryAssociations.next()) {
        // fill SELECTAssociation struct
        QueryBuilder::SelectAssociation selectAssociation;
        selectAssociation.strTable = queryAssociations.value(1).toString();
        selectAssociation.strColumn = queryAssociations.value(2).toString();
        selectAssociation.strJoinTable = queryAssociations.value(3).toString();
        selectAssociation.strJoinColumn = queryAssociations.value(4).toString();
        selectAssociation.enumRelation = queryAssociations.value(5).toString() == "hasOne" ? QueryBuilder::SelectAssociation::hasOne : (queryAssociations.value(5).toString() == "hasMany") ? QueryBuilder::SelectAssociation::hasMany : QueryBuilder::SelectAssociation::belongsTo;

        // save Association
        QueryBuilder::mapSelectAssociations.insert(selectAssociation.strTable, selectAssociation);
    }

    // initial done and everything is okay
    QueryBuilder::boolIsInitalized = true;
    return true;
}

QueryBuilder* QueryBuilder::initQuery(QueryBuilder::QueryType queryType)
{
    // construct query
    return new QueryBuilder(queryType);
}


//
// (De)Constructors
//

QueryBuilder::QueryBuilder(QueryBuilder::QueryType queryType)
{
    // save current query type
    this->currentQueryType = queryType;
    this->intLimit = -1;
}

//
// Build functions
//

QString QueryBuilder::toString(bool killMyself)
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

    // if user want to kill myself, so do it
    if(killMyself) {
        delete this;
    }

    return strQuery;
}

QSqlQuery QueryBuilder::toQuery(QString strDatabaseIntefier, bool killMyself)
{
    // if user want to kill myself, so do it
    QString strQuery = this->toString(false);
    if(killMyself) {
        delete this;
    }

    // construct sql query, if strDatabaseIntefier is empty, take the QueryBuilder static database intentifier
    return QSqlQuery(strQuery, QSqlDatabase::database(strDatabaseIntefier.isEmpty() ? QueryBuilder::strDatabaseIntefier : strDatabaseIntefier));
}


//
// Universal Query functions
//

QueryBuilder* QueryBuilder::Where(QString strTable, QString strField, QString strValue, bool isNumeric, QString strOpperator, QString strLogicOpperator, int level)
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
    condition.strLogicOpperator = strLogicOpperator;
    condition.isNumeric = isNumeric;
    this->mapWhereConditions.value(level)->append(condition);

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::OrderBy(QString strTable, QString strColumn, QueryBuilder::OrderType orderType)
{
    // only add group by field, if it will be selected!
    QString strOrderBy = QString("%1.%2").arg(strTable, strColumn);
    if(this->mapSelectExpressions.contains(strOrderBy)) {
       this->lstOrderBy.append(QString(" %1 %2").arg(strOrderBy, orderType == QueryBuilder::ASC ? "ASC" : "DESC"));
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::Limit(int limit)
{
    this->intLimit = limit;

    // at the end, return myself
    return this;
}


//
// Select Query functions
//

QueryBuilder* QueryBuilder::SelectTable(QString strTable, QString strTableAlias, QueryBuilder::JoinType joinTypeIfNeccessary)
{
    // exist if table doesn't exist
    if(!this->mapTableSchema.contains(strTable)) {
        return this;
    }

    // ...otherwise add table columns
    foreach(QString strColumnName, this->mapTableSchema.value(strTable).keys()) {
        QString strExpression = QString("%1.%2").arg(strTable, strColumnName);
        QString strAlias = QString("%1.%2").arg(strTableAlias.isEmpty() ? strTable : strTableAlias, strColumnName);
        this->mapSelectExpressions.insert(strExpression, strAlias);
    }

    // set from table if not allready set
    if(this->strFromTable.isEmpty()) {
        this->strFromTable = strTable;
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::SelectField(QString strTable, QString strColumn, QString strAlias, QueryBuilder::JoinType joinTypeIfNeccessary)
{
    // exit if table and column doesn't exist in database
    if(!this->mapTableSchema.contains(strTable) || !this->mapTableSchema.value(strTable).contains(strColumn)) {
        return this;
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

    // set from table if not allready set
    if(this->strFromTable.isEmpty()) {
        this->strFromTable = strTable;
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::From(QString strTable)
{
    // ony add table if it does exist
    if(QueryBuilder::mapTableSchema.contains(strTable)) {
        this->strFromTable = strTable;
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::Join(QString strTable, QueryBuilder::JoinType type)
{
    // only create join if join is defined in assioaction table
    if(this->mapSelectAssociations.contains(strTable)) {
        JoinData join;
        join.joinType = type;
        this->mapJoins.insert(strTable, join);
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::Join(QString strTableSrc, QString strTableTarget, QString strOnCondition, QueryBuilder::JoinType type)
{
    // add (blind) join
    JoinData join;
    join.strTargetTable = strTableTarget;
    join.joinType = type;
    join.strOnCondition = strOnCondition;
    this->mapJoins.insert(strTableSrc, join);

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::GroupBy(QString strTable, QString strColumn)
{
    // only add groupby Field if we select it!
    QString strGroupBy = QString("%1.%2").arg(strTable, strColumn);
    if(this->mapSelectExpressions.contains(QString("%1.%2").arg(strTable, strColumn))) {
        this->lstGroupBy.append(strGroupBy);
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::Having(QString strTable, QString strField, QString strValue, QString strOpperator, QString strLogicOpperator,  int level, bool isNumeric)
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
    condition.strLogicOpperator = strLogicOpperator;
    condition.isNumeric = isNumeric;
    this->mapHavingConditions.value(level)->append(condition);

    // at the end, return myself
    return this;
}


//
// Update Query functions
//

QueryBuilder* QueryBuilder::Update(QString strTable)
{
    // only set update table if it does exist
    if(this->mapTableSchema.contains(strTable)) {
        this->strUpdateTable = strTable;
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::UpdateField(QString strField, QString strValue, bool isNumeric)
{
    // only set update field if it does exist
    if(this->mapTableSchema.contains(this->strUpdateTable) || !this->mapTableSchema.value(this->strUpdateTable).contains(strField)) {
        this->lstUpdateFields.append(QString("%1 = %2").arg(strField, isNumeric ? strValue : '"' + strValue + '"'));
    }

    // at the end, return myself
    return this;
}


//
// Insert Query functions
//

QueryBuilder* QueryBuilder::Insert(QString strTable)
{
    // only set insert table if it does exist
    if(this->mapTableSchema.contains(strTable)) {
        this->strInsertTable = strTable;
    }

    // at the end, return myself
    return this;
}

QueryBuilder* QueryBuilder::InsertField(QString strFieldName, QString strValue, bool isNumeric)
{
    // only set insert field if it does exist
    if(this->mapTableSchema.contains(this->strInsertTable) || !this->mapTableSchema.value(this->strInsertTable).contains(strFieldName)) {
        this->mapInsertFields.insert(strFieldName, isNumeric ? strValue : '"'  + strValue + '"');
    }

    // at the end, return myself
    return this;
}

//
// Delete Query functions
//

QueryBuilder* QueryBuilder::Delete(QString strTable)
{
    // only set delete table if it does exist
    if(this->mapTableSchema.contains(strTable)) {
        this->strDeleteTable = strTable;
    }

    // at the end, return myself
    return this;
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
    bool firstJoin = true;
    foreach(QString strJoinTable, this->mapJoins.keys()) {
        // create string join
        JoinData join = this->mapJoins.value(strJoinTable);
        QString strJoinType = join.joinType == QueryBuilder::INNER ? "INNER" : (join.joinType == QueryBuilder::LEFT ? "LEFT" : "RIGHT");

        // join --> Associations
        if(join.strOnCondition.isEmpty() && this->mapSelectAssociations.contains(strJoinTable)) {
            foreach(QueryBuilder::SelectAssociation selectAssociation, this->mapSelectAssociations.values(strJoinTable)) {
                strJoin += QString(" %5 JOIN %1 ON %1.%2 = %3.%4 ").arg(selectAssociation.strTable, selectAssociation.strColumn, selectAssociation.strJoinTable, selectAssociation.strJoinColumn, strJoinType);
                firstJoin = false;
            }
        } else {
            strJoin += QString(" %1 %2 JOIN %3 ON (%4)").arg((firstJoin ? "" : strJoinTable), strJoinType, join.strTargetTable, join.strOnCondition);
            firstJoin = false;
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
            strWhere += QString("%1.%2 %4 %3").arg(condition.strTable, condition.strColumn, condition.isNumeric ? condition.strValue : '"' + condition.strValue + '"', condition.strLogicOpperator);
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
