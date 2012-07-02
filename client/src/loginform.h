/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef LOGINFORM_H
#define LOGINFORM_H

// Qt (core)
#include <QtCore/QTimer>
#include <QtCore/QCryptographicHash>

// Qt (gui)
#include <QtGui/QMainWindow>

// own (protbuf)
#include "protocol.pb.h"

// own (client)
#include "global.h"

// own (gui)
#include "mainwindow.h"

namespace Ui {
    class LoginForm;
}

class LoginForm : public QMainWindow
{
    Q_OBJECT

    public:
        LoginForm(QString strErrorMessage = "", QWidget *parent = 0);
        ~LoginForm();

    private:
        Ui::LoginForm *ui;

    private slots:
        // Gui
        bool loginValidator();
        void login();

        // PacketProcessor
        void loginRequestReceived(bool loggedin);

        // Socket slots
        void serverConnectionSuccessfull();
        void serverConnectionError(QAbstractSocket::SocketError socketError);
        void connectToServer();

};

#endif // LOGINFORM_H
