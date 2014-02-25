/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "global.h"

// init static vars
QTcpSocket* Global::socketServer = 0;
EleaphRpc* Global::eleaphRpc = 0;
bool Global::init = false;
bool Global::boolLoggedIn = false;
QMap<qint32, Protocol::Contact*> Global::mapContactList;
QMap<qint32, Protocol::User*> Global::mapCachedUsers;
Protocol::User* Global::user = 0;
QSettings* Global::settings = 0;

// Fixme: make it dynamic with a config file
QString Global::strServerHostname = "localhost";
quint16 Global::intServerPort = 1234;

QString Global::strSessionName = QHostInfo::localHostName();

void Global::initialize()
{
    // stop init process and exit if the class was allready initialized
    if(Global::init) {
        return;
    }

    // simplefy application instance
    QCoreApplication* app = QApplication::instance();


    // init settings
    QString strConfigFile = QFileInfo(app->applicationFilePath()).baseName() + ".ini";
    if(!QFile::exists(strConfigFile)) {
        qFatal("Cannot find Config file:\r\n%s", qPrintable(strConfigFile));
    }
    Global::settings = new QSettings(strConfigFile, QSettings::IniFormat);


    Global::strServerHostname = Global::settings->value("server/hostname", Global::strServerHostname).toString();
    Global::intServerPort = Global::settings->value("server/port", Global::intServerPort).toUInt();
    Global::strSessionName = Global::settings->value("session/name", Global::strSessionName).toString();


    // init socket
    Global::socketServer = new QTcpSocket(app);

    // init EleaphRpc handler
    Global::eleaphRpc = new EleaphRpc(app);
    Global::eleaphRpc->addDevice(Global::socketServer, IEleaph::NeverForgetDevice);

    // class was successfull initialized!
    Global::init = true;
}
