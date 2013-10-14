/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

// Qt (core)
#include <QtGui/QApplication>

// own (gui)
#include "loginform.h"

// own (client)
#include "global.h"

int main(int argc, char *argv[])
{
    // init qt
    QApplication a(argc, argv);

    // init global class
    Global::initialize();

    // init login form
    LoginForm *formLogin = new LoginForm;
    formLogin->show();

    // start eventloop
    return a.exec();
}
