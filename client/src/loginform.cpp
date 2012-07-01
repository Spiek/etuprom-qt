/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

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
    this->connectToServer();
}

LoginForm::~LoginForm()
{
    delete ui;
}


bool LoginForm::loginValidator()
{
    if(this->ui->lineEditLogin->text().isEmpty() || this->ui->lineEditPassword->text().isEmpty()) {
        this->ui->pushButtonLogin->setEnabled(false);
        return false;
    } else {
        this->ui->pushButtonLogin->setEnabled(true);
        return true;
    }
}

void LoginForm::login()
{
    if(!this->loginValidator()) {
        return;
    }

    this->ui->pushButtonLogin->setEnabled(false);
    this->ui->statusbar->showMessage("Login...");

    // create login protobuf objects and send it to the server
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    Protocol::Packet packet;
    packet.set_packettype(Protocol::Packet_PacketType_LoginRequest);

    Protocol::LoginRequest *login = packet.mutable_requestlogin();
    login->set_username(this->ui->lineEditLogin->text().toUtf8().constData());
    login->set_password(QCryptographicHash::hash(this->ui->lineEditPassword->text().toAscii(), QCryptographicHash::Sha1).toHex().constData());
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
    this->ui->statusbar->showMessage("Successfull connected to Server...", 10000);
    this->ui->centralwidget->setDisabled(false);
}

void LoginForm::serverConnectionError(QAbstractSocket::SocketError socketError)
{
    this->ui->statusbar->showMessage(QString("Error \"%1\" occours, try again in 10 Seconds...").arg(Global::socketServer->errorString()));
    this->ui->centralwidget->setDisabled(true);
    QTimer::singleShot(10000, this, SLOT(connectToServer()));
}

void LoginForm::loginRequestReceived(bool loggedin)
{
    // if login wasn't success
    if(!loggedin) {
        this->ui->statusbar->showMessage("Login failed, username or password wrong, please try again...");
        this->ui->lineEditPassword->clear();
        this->ui->centralwidget->setDisabled(false);
    } else {
        MainWindow *mainWindow = new MainWindow;
        mainWindow->show();
        this->deleteLater();
    }
}
