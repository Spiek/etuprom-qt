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

    // reset log in state
    Global::boolLoggedIn = false;

    // if error message was set, set error message
    if(!strErrorMessage.isEmpty()) {
        this->ui->statusbar->showMessage(strErrorMessage);
    }

    // only allow unconnected sockets, reset all other socket states to have a clear state
    if(!(Global::socketServer->state() == QTcpSocket::UnconnectedState || Global::socketServer->state() == QTcpSocket::ConnectedState)) {
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
    Global::eleaphRpc->registerRPCMethod("login", this, SLOT(loginResponse(DataPacket*)));

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
    // create packet LoginRequest
    Protocol::LoginRequest requestLogin;
    requestLogin.set_username(this->ui->lineEditLogin->text().toStdString());
    QByteArray baPasswordHash = QCryptographicHash::hash(this->ui->lineEditPassword->text().toUtf8(), QCryptographicHash::Sha1).toHex();
    requestLogin.set_password(baPasswordHash.data());

    // send protobuf packet container as Length-Prefix-packet over the stream
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, "login", requestLogin.SerializeAsString());
}


void LoginForm::connectToServer()
{
     // don't connect to server if allready connected
    if(Global::socketServer->state() == QTcpSocket::ConnectedState) {
        return this->ui->centralwidget->setDisabled(false);
    }

    // connect to server, and inform the user about it
    Global::socketServer->connectToHost(Global::strServerHostname, Global::intServerPort);
    this->ui->statusbar->showMessage("Try to connect to Server...");
    this->ui->centralwidget->setDisabled(true);
}


void LoginForm::serverConnectionSuccessfull()
{
    // inform the user about the successfull connection and enable the window, so that the user can login!
    this->ui->statusbar->showMessage("Successfull connected to Server... Ready for login");
    this->ui->centralwidget->setDisabled(false);
}

void LoginForm::serverConnectionError(QAbstractSocket::SocketError socketError)
{
    // inform the user about the error and not allow the user to login
    this->ui->centralwidget->setDisabled(true);
    this->ui->statusbar->showMessage(QString("Error \"%1\" occours, try again in 10 Seconds...").arg(Global::socketServer->errorString()));

    // try to establish the connection to the server after 3 seconds again
    QTimer::singleShot(10000, this, SLOT(connectToServer()));
}

void LoginForm::loginResponse(DataPacket *dataPacket)
{
    Protocol::LoginResponse responseLogin;
    responseLogin.ParseFromArray(dataPacket->baRawPacketData->data(), dataPacket->baRawPacketData->length());

    // inform the user about login wasn't success, reset the password, and let the user login again :-)
    if(responseLogin.type() == Protocol::LoginResponse_Type_LoginIncorect) {
        this->ui->statusbar->showMessage("Login failed, username or password wrong, please try again...");
        this->ui->centralwidget->setDisabled(false);
    }

    // login was success, close login form and jump to MainWindow
    else {
        Global::boolLoggedIn = true;
        MainWindow* mainWindow = new MainWindow;
        mainWindow->show();
        this->deleteLater();
    }
}
