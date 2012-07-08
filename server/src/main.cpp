/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

// Qt (core)
#include <QtCore/QCoreApplication>

// Qt (sql)
#include <QtSql/QSqlDatabase>

// own libs
#include "global.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // initialize globalization
    Global::initialize();

    // start eventloop
    return a.exec();
}
