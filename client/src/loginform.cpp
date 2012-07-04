/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QString strErrorMessage, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    // if error message was set, set error message and reconstruct server socket
    if(!strErrorMessage.isEmpty()) {
        this->ui->statusbar->showMessage(strErrorMessage);
    }

    // only allow unconnected sockets, reset all other socket states to have a clear state
    if(Global::socketServer->state() !=  QTcpSocket::UnconnectedState) {
        Global::socketServer->disconnectFromHost();
    }

    // signal --> slot connections (Socket)
    this->connect(Global::socketServer, SIGNAL(connected()), this, SLOT(serverConnectionSuccessfull()));
    this->connect(Global::socketServer, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverConnectionError(QAbstractSocket::SocketError)));

    // signal --> slot connections (Gui)
    this->connect(this->ui->lineEditLogin, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));
    this->connect(this->ui->lineEditPassword, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));
    this->connect(this->ui->lineEditLogin, SIGNAL(returnPressed()), this, SLOT(login()));
    this->connect(this->ui->lineEditPassword, SIGNAL(returnPressed()), this, SLOT(login()));
    this->connect(this->ui->pushButtonLogin, SIGNAL(clicked()), this, SLOT(login()));

    // signal --> slot connections (PacketProcessor)
    this->connect(Global::packetProcessor, SIGNAL(loginResponse(bool)), this, SLOT(loginRequestReceived(bool)));

    // connect to server
    // Note: it could happen that the class was constructed in an event from the QTcpSocket (like the QTcpSocket::error() slot),
    //       if this is the case, a "direct" host reconnect doesn't work, to solve this issue we call the connectToServer slot over the event system (not directly!)
    //       src: https://bugreports.qt-project.org/browse/QTBUG-18082
    QMetaObject::invokeMethod(this, "connectToServer", Qt::QueuedConnection);
}

LoginForm::~LoginForm()
{
    delete ui;
}


bool LoginForm::loginValidator()
{
    // enable the login button if username and password was typed in
    if(this->ui->lineEditLogin->text().isEmpty() || this->ui->lineEditPassword->text().isEmpty()) {
        this->ui->pushButtonLogin->setEnabled(false);
        return false;
    }

    // otherwise disable the login button
    else {
        this->ui->pushButtonLogin->setEnabled(true);
        return true;
    }
}

void LoginForm::login()
{
    // disbale the login button, so that the user can't login twice
    this->ui->pushButtonLogin->setEnabled(false);

    // inform the user about the login process
    this->ui->statusbar->showMessage("Login...");

    // create login protobuf objects and send it to the server
    // create packet container
    Protocol::Packet packet;
    packet.set_packettype(Protocol::Packet_PacketType_LoginRequest);

    // create login packet
    Protocol::LoginRequest *login = packet.mutable_requestlogin();
    login->set_username(this->ui->lineEditLogin->text().toUtf8().constData());
    login->set_password(QCryptographicHash::hash(this->ui->lineEditPassword->text().toAscii(), QCryptographicHash::Sha1).toHex().constData());

    // send protobuf packet container as Length-Prefix-packet over the stream
    Global::packetHandler->sendDataPacket(Global::socketServer, packet.SerializeAsString());
}


void LoginForm::connectToServer()
{
     // connect to server
    Global::socketServer->connectToHost(Global::strServerHostname, Global::intServerPort);

    // inform the user
    this->ui->statusbar->showMessage("Try to connect to Server...");
    this->ui->centralwidget->setDisabled(true);
}


void LoginForm::serverConnectionSuccessfull()
{
    // inform the user about the successfull connection and enable the window, so that the user can login!
    this->ui->statusbar->showMessage("Successfull connected to Server...", 10000);
    this->ui->centralwidget->setDisabled(false);
}

void LoginForm::serverConnectionError(QAbstractSocket::SocketError socketError)
{
    // inform the user about the error and not allow the user to login
    this->ui->statusbar->showMessage(QString("Error \"%1\" occours, try again in 3 Seconds...").arg(Global::socketServer->errorString()));
    this->ui->centralwidget->setDisabled(true);

    // try to establish the connection to the server after 3 seconds again
    QTimer::singleShot(3000, this, SLOT(connectToServer()));
}

void LoginForm::loginRequestReceived(bool loggedin)
{
    // inform the user about login wasn't success, reset the password, and let the user login again :-)
    if(!loggedin) {
        this->ui->statusbar->showMessage("Login failed, username or password wrong, please try again...");
        this->ui->lineEditPassword->clear();
        this->ui->centralwidget->setDisabled(false);
    }

    // login was success, close login form and jump to MainWindow
    else {
        MainWindow *mainWindow = new MainWindow;
        mainWindow->show();
        this->deleteLater();
    }
}
