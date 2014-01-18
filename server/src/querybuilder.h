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
    public:
        // static data structures
        enum QueryType {
            SELECT = 1,
            UPDATE = 2,
            INSERT = 3,
            DELETE = 4
        };
        enum JoinType {
            NONE = 0,
            INNER = 1,
            LEFT = 2,
            RIGHT = 3
        };
        enum OrderType {
            ASC = 0,
            DESC = 1
        };

    private:
        // static data structures
        struct JoinData {
            JoinType joinType;
            QString strTargetTable;
            QString strOnCondition;
        };

        struct SelectAssociation {
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
            QString strLogicOpperator;
        };

    public:
        // static implementations
        static bool initialize(QString strDatabaseIntefier);
        static QueryBuilder* initQuery(QueryBuilder::QueryType queryType = QueryBuilder::SELECT);

    private :
        // static global data
        static QMap<QString, QMap<QString, QVariant::Type> > mapTableSchema;
        static QMap<QString, QueryBuilder::SelectAssociation> mapSelectAssociations;
        static QString strDatabaseIntefier;
        static bool boolIsInitalized;


        QueryBuilder(QueryBuilder::QueryType queryType);

   public:
        // Query construction functions
        QString toString(bool killMyself = true);
        QSqlQuery toQuery(QString strDatabaseIntefier = "", bool killMyself = true);

        // Universal Query functions
        QueryBuilder* Where(QString strTable, QString strField, QString strValue, bool isNumeric = false, QString strOpperator = "AND", QString strLogicOpperator = "=", int level = 1);
        QueryBuilder* OrderBy(QString strTable, QString strColumn, QueryBuilder::OrderType orderType = QueryBuilder::ASC);
        QueryBuilder* Limit(int limit = -1);

        // Select Query functions
        QueryBuilder* SelectTable(QString strTable, QString strTableAlias = "");
        QueryBuilder* SelectField(QString strTable, QString strColumn, QString strAlias = "");
        QueryBuilder* From(QString strTable);
        QueryBuilder* Join(QString strTable, QueryBuilder::JoinType type = QueryBuilder::INNER);
        QueryBuilder* Join(QString strTableSrc, QString strTableTarget, QString strOnCondition, QueryBuilder::JoinType type = QueryBuilder::INNER);
        QueryBuilder* GroupBy(QString strTable, QString strColumn);
        QueryBuilder* Having(QString strTable, QString strField, QString strValue, QString strOpperator = "AND", QString strLogicOpperator = "=",  int level = 1, bool isNumeric = false);

        // Update Query functions
        QueryBuilder* Update(QString strTable);
        QueryBuilder* UpdateField(QString strField, QString strValue, bool isNumeric = false);

        // Insert Query functions
        QueryBuilder* Insert(QString strTable);
        QueryBuilder* InsertField(QString strFieldName, QString strValue, bool isNumeric = false);

        // Delete Query functions
        QueryBuilder* Delete(QString strTable);

    private:
        // Global Data
        QueryBuilder::QueryType currentQueryType;

        // Universal Query Data
        QMap<int, QList<QueryBuilder::Condition>*> mapWhereConditions;

        // Select Data
        QString strFromTable;
        QMap<QString, QueryBuilder::JoinData> mapJoins;
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
