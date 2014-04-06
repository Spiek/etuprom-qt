/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QString strErrorMessage, QWidget *parent) :
    QMainWindow(parent),
    controller(this),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    // init protocol controller events
    this->connect(&this->controller, SIGNAL(connectionChanged(QTcpSocket*,LoginProtocolController::ConnectionState)), this, SLOT(connectionChanged(QTcpSocket*,LoginProtocolController::ConnectionState)));

    // some style improvings
    this->loadDesign("default");
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size());
    this->ui->lineEditSession->setText(Global::strSessionName);

    // if error message was set, set error message
    if(!strErrorMessage.isEmpty()) {
        this->ui->statusbar->showMessage(strErrorMessage);
    }

    // signal --> slot connections (Gui)
    this->connect(this->ui->lineEditLogin, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));
    this->connect(this->ui->lineEditPassword, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));
    this->connect(this->ui->lineEditSession, SIGNAL(textChanged(QString)), this, SLOT(loginValidator()));

    this->connect(this->ui->lineEditLogin, SIGNAL(returnPressed()), this, SLOT(login()));
    this->connect(this->ui->lineEditPassword, SIGNAL(returnPressed()), this, SLOT(login()));
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


void LoginForm::connectionChanged(QTcpSocket *socketServer, LoginProtocolController::ConnectionState connectionState)
{
    // successfull connect --> enable login
    if(connectionState == LoginProtocolController::ConnectionState::Connected) {
        this->ui->statusbar->showMessage("Successfull connected to Server... Ready for login");
        this->ui->centralwidget->setDisabled(false);
    }

    // connection error --> inform user and try again
    else if(connectionState == LoginProtocolController::ConnectionState::ConnectionError) {
        this->ui->centralwidget->setDisabled(true);
        this->ui->statusbar->showMessage(QString("Error \"%1\" occours, try again in 3 Seconds...").arg(socketServer->errorString()));

        // try to establish the connection to the server after 3 seconds again
        QTimer::singleShot(3000, this, SLOT(connectToServer()));
    }

    // disconnect --> reconnect
    else if(connectionState == LoginProtocolController::ConnectionState::Disconnected) {
        this->ui->statusbar->showMessage("Server disconnected, try reconnect again in 3 Seconds");
        QTimer::singleShot(3000, this, SLOT(connectToServer()));
    }
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

    // prepare gui for connection
    this->ui->centralwidget->setDisabled(true);
    this->ui->statusbar->showMessage("Try to connect to Server...");
    this->controller.connectToServer();
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
    // handle double clicks by user on login button!
    if(!this->ui->pushButtonLogin->isEnabled()) {
        return;
    }
    this->ui->pushButtonLogin->setEnabled(false);

    // inform the user about the login process
    this->ui->statusbar->showMessage("Login...");

    // create login protobuf objects
    Protocol::LoginResponse responseLogin = this->controller.login(this->ui->lineEditLogin->text(), this->ui->lineEditPassword->text(), this->ui->lineEditSession->text());

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
    // get own user (exit if error occours!)
    this->ui->statusbar->showMessage("Get Userdata...");
    Protocol::User *user = this->controller.fetchOwnUserData();
    if(!user) {
        return;
    }
    Global::user = user;


    // get contact list
    this->ui->statusbar->showMessage("Get Contactlist...");
    this->controller.fetchContactList();

    // jump to main form (and destroy login form!)
    (new MainWindow)->show();
    this->deleteLater();
}
