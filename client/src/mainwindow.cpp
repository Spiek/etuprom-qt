/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // signal --> slot connections (Socket)
    this->connect(Global::socketServer, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverConnectionError(QAbstractSocket::SocketError)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::serverConnectionError(QAbstractSocket::SocketError socketError)
{
    // if client disconnect from server, inform the user about and jump back to login form
    this->deleteLater();
    QString strErrorMessage = QString("Error \"%1\" occours, please relog...").arg(Global::socketServer->errorString());
    LoginForm *loginForm = new LoginForm(strErrorMessage);
    loginForm->show();
}
