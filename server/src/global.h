/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#ifndef GLOBAL_H
#define GLOBAL_H

// Qt (core)
#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtCore/QFile>

// own libs
#include "databasehelper.h"

class Global
{
    public:
        // global initializer
        static void initialize();

        // global object getter
        static DatabaseHelper* getDatabaseHelper();

        // global config handlers
        static QVariant getConfigValue(QString strKey, QVariant varDefault = "", bool boolSync = false);
        static void setConfigValue(QString strKey, QVariant varValue, bool boolSync = true);

    private:
        // settings
        static QSettings *settings;
        static bool init;

        // SINGELTON objects
        static DatabaseHelper* databaseHelper;
};

#endif // GLOBAL_H
