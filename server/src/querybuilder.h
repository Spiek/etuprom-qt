/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#ifndef QUERYBUILDER_H
#define QUERYBUILDER_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>

class QueryBuilder
{
    // static implementations
    public:
        enum QueryType {
            SELECT = 1,
            UPDATE = 2,
            INSERT = 3,
            DELETE = 4
        };
        enum JoinType {
            INNER = 1,
            LEFT = 2,
            RIGHT = 3
        };
        enum OrderType {
            ASC = 0,
            DESC = 1
        };

    private:
        struct SELECTAssociation {
            QString strTable;
            QString strColumn;
            QString strJoinTable;
            QString strJoinColumn;
            enum Relation {
                hasOne = 0,
                hasMany = 1,
                belongsTo = 2
            } enumRelation;
        };
        struct Condition {
            QString strTable;
            QString strColumn;
            QString strValue;
            bool isNumeric;
            QString strOpperator;
        };

    public:
        QueryBuilder(QString strDatabaseIntefier);

        // Query functions
        void updateTableSchemaCache();
        void createNewQuery(QueryBuilder::QueryType queryType = QueryBuilder::SELECT);
        void clear(QueryBuilder::QueryType newQueryType = QueryBuilder::SELECT);
        QString buildQuery();

        // Universal Query functions
        void addWhereCondition(QString strTable, QString strField, QString strValue, bool isNumeric = false, QString strOpperator = "AND", int level = 1);
        bool addOrderBy(QString strTable, QString strColumn, QueryBuilder::OrderType orderType = QueryBuilder::ASC);
        void setLimit(int limit = -1);

        // Select Query functions
        bool addSelectExpressionTable(QString strTable, QString strTableAlias = "", QueryBuilder::JoinType joinTypeIfNeccessary = QueryBuilder::INNER);
        bool addSelectExpression(QString strTable, QString strColumn, QString strAlias = "", QueryBuilder::JoinType joinTypeIfNeccessary = QueryBuilder::INNER);
        bool addSelectJoin(QString strTable, QueryBuilder::JoinType type = QueryBuilder::INNER);
        bool addSelectGroupBy(QString strTable, QString strColumn);
        void addSelectHavingCondition(QString strTable, QString strField, QString strValue, QString strOpperator = "AND", int level = 1, bool isNumeric = false);

        // Update Query functions
        bool setUpdateTable(QString strTable);
        bool addUpdateField(QString strField, QString strValue, bool isNumeric = false);

        // Insert Query functions
        bool setInsertTable(QString strTable);
        bool setInsertField(QString strFieldName, QString strValue, bool isNumeric = false);

        // Delete Query functions
        bool setDeleteTable(QString strTable);

    private:
        // Global Data
        QMap<QString, QMap<QString, QVariant::Type> > mapTableSchema;
        QString strDatabaseIntefier;
        QueryBuilder::QueryType currentQueryType;

        // Universal Query Data
        QMap<int, QList<QueryBuilder::Condition>*> mapWhereConditions;

        // Select Data
        QString strFromTable;
        QMap<QString, QueryBuilder::JoinType> mapJoins;
        QMap<QString, QueryBuilder::SELECTAssociation> mapSelectAssociations;
        QMap<QString, QString> mapSelectExpressions;
        QStringList lstGroupBy;
        QMap<int, QList<QueryBuilder::Condition>*> mapHavingConditions;
        QStringList lstOrderBy;
        int intLimit;

        // Update data
        QString strUpdateTable;
        QStringList lstUpdateFields;

        // Insert data
        QString strInsertTable;
        QMap<QString, QString> mapInsertFields;

        // Delete data
        QString strDeleteTable;

        // Global - Construction Methods
        void constructQueryPart_Condition(QString* strQuery, QMap<int, QList<QueryBuilder::Condition>*>* mapConditions);
        void constructQuery_Select_OrderBy(QString* strQuery);

        // Select - Construction Methods
        void constructQuery_Select(QString* strQuery);
        void constructQuery_Select_GroupBy(QString* strQuery);

        // Update - Construction Methods
        void constructQuery_Update(QString* strQuery);

        // Insert - Construction Methods
        void constructQuery_Insert(QString* strQuery);

        // Delete - Construction Methods
        void constructQuery_Delete(QString* strQuery);
};

#endif // QUERYBUILDER_H
