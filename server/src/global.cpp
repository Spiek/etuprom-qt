/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
 
#include "global.h"

// init static vars
DatabaseHelper* Global::databaseHelper = 0;
QSettings* Global::settings = 0;

bool Global::init = false;

void Global::initialize()
{
    // stop init process and exit if the class was allready initialized
    if(Global::init) {
        return;
    }

    // simplefy application instance
    QCoreApplication* app = QCoreApplication::instance();

    /// init settings
    QString strConfigFile = QFileInfo(app->applicationFilePath()).baseName() + ".ini";
    if(!QFile::exists(strConfigFile)) {
        qFatal("Cannot find Config file:\r\n%s", qPrintable(strConfigFile));
    }
    Global::settings = new QSettings(strConfigFile, QSettings::IniFormat);

    /// init database helper
    QSqlDatabase database = QSqlDatabase::addDatabase(Global::getConfigValue("database/driver").toString(), "sqm");
    database.setDatabaseName(Global::getConfigValue("database/name", "").toString());
    database.setHostName(Global::getConfigValue("database/hostname", "").toString());
    database.setPort(Global::getConfigValue("database/port", 0).toInt());
    database.setUserName(Global::getConfigValue("database/username", "").toString());
    database.setPassword(Global::getConfigValue("database/password", "").toString());
    database.setConnectOptions(Global::getConfigValue("database/connectoptions", "").toString());
    if(database.open()) {
        printf("Datenbank Verbindung erfolgreich!");
    } else {
        qFatal("Datenbank Verbindung NICHT erfolgreich!");
    }
    Global::databaseHelper = new DatabaseHelper("sqm");

    // class was successfull initialized!
    Global::init = true;
}

DatabaseHelper* Global::getDatabaseHelper()
{
    return Global::databaseHelper;
}


QVariant Global::getConfigValue(QString strKey, QVariant varDefault, bool boolSync)
{
    // sync before read, if user want it
    if(boolSync) {
        Global::settings->sync();
    }

    // return needed value
    return Global::settings->value(strKey, varDefault);
}

void Global::setConfigValue(QString strKey, QVariant varValue, bool boolSync)
{
    // set the value
    Global::settings->setValue(strKey, varValue);

    // sync after write, if user want it
    if(boolSync) {
        Global::settings->sync();
    }
}
