/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QtGui/QMainWindow>
#include <QtCore/QTimer>
#include <QtCore/QCryptographicHash>

#include "global.h"
#include "protocol.pb.h"
#include "mainwindow.h"

namespace Ui {
    class LoginForm;
}

class LoginForm : public QMainWindow
{
    Q_OBJECT

    public:
        LoginForm(QWidget *parent = 0);
        ~LoginForm();

    private:
        Ui::LoginForm *ui;

    private slots:
        // gui
        bool loginValidator();
        void login();
        void loginRequestReceived(bool loggedin);

        // server
        void serverConnectionSuccessfull();
        void serverConnectionError(QAbstractSocket::SocketError socketError);
        void connectToServer();

};

#endif // LOGINFORM_H
