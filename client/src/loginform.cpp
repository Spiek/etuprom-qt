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

    // some style improvings
    this->loadDesign("default");
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size());
    this->ui->lineEditSession->setText(Global::strSessionName);

    // if error message was set, set error message
    if(!strErrorMessage.isEmpty()) {
        this->ui->statusbar->showMessage(strErrorMessage);
    }

    // only allow unconnected sockets, reset all other socket states to have a clear state
    if(!(Global::socketServer->state() == QTcpSocket::UnconnectedState || Global::socketServer->state() == QTcpSocket::ConnectedState)) {
        Global::socketServer->disconnectFromHost();
    }

    // signal --> slot connections (Socket)
    this->connect(Global::socketServer, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));

    // signal --> slot connections (Gui)
    this->connect(this->ui->lineEditLogin, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));
    this->connect(this->ui->lineEditPassword, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));
    this->connect(this->ui->lineEditSession, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));

    this->connect(this->ui->lineEditLogin, SIGNAL(returnPressed()), this, SLOT(login()));
    this->connect(this->ui->lineEditPassword, SIGNAL(returnPressed()), this, SLOT(login()));
    this->connect(this->ui->lineEditSession, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));

    this->connect(this->ui->pushButtonLogin, SIGNAL(clicked()), this, SLOT(login()));

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


//
// Server Connection Signal handling
//

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

    // make a syncron connect
    QEventLoop loop;
    loop.connect(Global::socketServer, SIGNAL(error(QAbstractSocket::SocketError)), &loop, SLOT(quit()));
    loop.connect(Global::socketServer, SIGNAL(connected()), &loop, SLOT(quit()));
    loop.exec();

    // if socket is connected, we have a successfull connect
    if(Global::socketServer->state() == QTcpSocket::ConnectedState) {
        // inform the user about the successfull connection and enable the window, so that the user can login!
        this->ui->statusbar->showMessage("Successfull connected to Server... Ready for login");
        this->ui->centralwidget->setDisabled(false);
    }

    // ...otherwise we have an connection error, so inform user and try again
    else {
        // inform the user about the error and not allow the user to login
        this->ui->centralwidget->setDisabled(true);
        this->ui->statusbar->showMessage(QString("Error \"%1\" occours, try again in 10 Seconds...").arg(Global::socketServer->errorString()));

        // try to establish the connection to the server after 10 seconds again
        QTimer::singleShot(10000, this, SLOT(connectToServer()));
    }
}

void LoginForm::serverDisconnected()
{
    this->ui->statusbar->showMessage("Server disconnected, try reconnect again in 3 Seconds");
    QTimer::singleShot(3000, this, SLOT(connectToServer()));
}


//
// Style
//

void LoginForm::loadDesign(QString strDesign)
{
    // set stylesheet
    QFile file(QString("design/%1/Login/form.css").arg(strDesign));
    file.open(QFile::ReadOnly);
    this->ui->centralwidget->setStyleSheet(file.readAll());
}


//
// Login
//

bool LoginForm::loginValidator()
{
    // not valid if the username or password or session was not typed in
    bool isValid =  !(
                        this->ui->lineEditLogin->text().isEmpty() ||
                        this->ui->lineEditPassword->text().isEmpty() ||
                        this->ui->lineEditSession->text().isEmpty()
                    );

    this->ui->pushButtonLogin->setEnabled(isValid);
    return isValid;
}

void LoginForm::login()
{
    // disbale the login button, so that the user can't perform a login twice
    this->ui->pushButtonLogin->setEnabled(false);

    // inform the user about the login process
    this->ui->statusbar->showMessage("Login...");

    // create login protobuf objects
    Protocol::LoginRequest requestLogin;
    requestLogin.set_username(this->ui->lineEditLogin->text().toStdString());
    QByteArray baPasswordHash = QCryptographicHash::hash(this->ui->lineEditPassword->text().toUtf8(), QCryptographicHash::Sha1).toHex();
    requestLogin.set_password(baPasswordHash.data());
    requestLogin.set_sessionname(this->ui->lineEditSession->text().toStdString());

    // ... and send constructed protobuf packet to the server
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, PACKET_DESCRIPTOR_USER_LOGIN, requestLogin.SerializeAsString());

    // wait for packet async and process it
    EleaphRpcPacket epLoginResponse = Global::eleaphRpc->waitAsyncForPacket(PACKET_DESCRIPTOR_USER_LOGIN);

    // parse server response and handle it
    Protocol::LoginResponse responseLogin;
    responseLogin.ParseFromArray(epLoginResponse.data()->baRawPacketData->data(), epLoginResponse.data()->baRawPacketData->length());

    // inform the user about login wasn't success, reset the password, and let the user login again :-)
    if(responseLogin.type() == Protocol::LoginResponse_Type_LoginIncorect) {
        this->ui->statusbar->showMessage("Login failed, username or password wrong, please try again...");
        this->ui->centralwidget->setDisabled(false);
    }

    else if(responseLogin.type() == Protocol::LoginResponse_Type_AllreadyLoggedIn) {
        this->ui->statusbar->showMessage("Login failed, only one login is allowed...");
        this->ui->centralwidget->setDisabled(false);
    }

    // login was success, get now all needed extra informations for main form
    else {
        this->ui->statusbar->showMessage("Login Success...");
        return getNeededDataForMainForm();
    }
}

void LoginForm::getNeededDataForMainForm()
{
    ///
    /// get own user data
    ///
    // request own user data from server
    this->ui->statusbar->showMessage("Get Userdata...");
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, PACKET_DESCRIPTOR_USER_SELF_GET_INFO);
    EleaphRpcPacket epUserSelf = Global::eleaphRpc->waitAsyncForPacket(PACKET_DESCRIPTOR_USER_SELF_GET_INFO);

   // parse and handle own user data
    Protocol::User *user = new Protocol::User;
    if(!user->ParseFromArray(epUserSelf.data()->baRawPacketData->constData(), epUserSelf.data()->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse User", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }

    // save user
    Global::user = user;


    ///
    /// get contact list
    ///
    // request contact list from server
    this->ui->statusbar->showMessage("Get Contactlist...");
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, PACKET_DESCRIPTOR_CONTACT_GET_LIST);
    EleaphRpcPacket epContactList = Global::eleaphRpc->waitAsyncForPacket(PACKET_DESCRIPTOR_CONTACT_GET_LIST);

    // if no contact list was given skip contact list handling
    Global::mapContactList.clear();
    if(epContactList.data()->baRawPacketData->length() > 0)  {
        // parse and handle contact list
        Protocol::ContactList contactList;
        if(!contactList.ParseFromArray(epContactList.data()->baRawPacketData->constData(), epContactList.data()->baRawPacketData->length())) {
            qWarning("[%s][%d] - Protocol Violation by Trying to Parse User", __PRETTY_FUNCTION__ , __LINE__);
            return;
        }

        // save all contacts for global access

        for(int i = 0; i < contactList.contact_size(); i++) {
            Protocol::Contact* contact = new Protocol::Contact(contactList.contact(i));
            Global::mapContactList.insert(contact->user().id(), contact);
            Global::mapCachedUsers.insert(contact->user().id(), contact->mutable_user());
        }
    }

    // jump to main form (and destroy login form!)
    this->deleteLater();
    MainWindow *mw = new MainWindow;
    mw->show();
}
