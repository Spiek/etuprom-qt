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

    // signal --> slot connections (PacketProcessor)
    Global::eleaphRpc->registerRPCMethod(PACKET_DESCRIPTOR_CONTACT_ALTERED, this, SLOT(handleUserAltered(DataPacket*)));

    // handle double click event of new user
    this->connect(this->ui->treeWidgetContactList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onUserClicked(QTreeWidgetItem*,int)));
    this->connect(this->ui->actionLogout, SIGNAL(triggered()), this, SLOT(handleLogout()));
    this->connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(deleteLater()));

    // create contact list
    this->constructContactList();

    this->chatBox = new ChatBox;
    this->chatBox->loadDesign("Orchid");

    // setup main user
    this->setupLoggedInUser();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->chatBox;
}


void MainWindow::setupLoggedInUser()
{
    // set user data
    this->ui->labelUsername->setText(QString::fromStdString(Global::user->username()));
}

void MainWindow::serverConnectionError(QAbstractSocket::SocketError socketError)
{
    // if client disconnect from server, inform the user about and jump back to login form
    Q_UNUSED(socketError);
    this->deleteLater();
    QString strErrorMessage = QString("Error \"%1\" occours, please relog...").arg(Global::socketServer->errorString());
    LoginForm *loginForm = new LoginForm(strErrorMessage);
    loginForm->show();
}

void MainWindow::constructContactList()
{
    // remove all users form contact list
    this->ui->treeWidgetContactList->clear();

    // loop contacts in contact list and construct all needed gui elements
    foreach(Protocol::Contact* contact, Global::mapContactList.values()) {
        this->setupUser(contact->mutable_user(), QString::fromStdString(contact->group()));
    }

    // expand all groups
    this->ui->treeWidgetContactList->expandAll();
}

void MainWindow::handleUserAltered(EleaphRpcPacket dataPacket)
{
    Protocol::User user;
    user.ParseFromArray(dataPacket.data->baRawPacketData->data(), dataPacket.data->baRawPacketData->length());
    this->contactListUserAltered(user);
}

void MainWindow::handleLogout()
{
    // cleanup all variable global data
    qDeleteAll(Global::mapContactList.values());
    Global::mapContactList.clear();
    Global::mapCachedUsers.clear();

    // process logout
    Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, "logout");
    this->deleteLater();
    LoginForm *loginForm = new LoginForm("Successfull logged out...");
    loginForm->show();
}



void MainWindow::contactListUserAltered(Protocol::User user)
{
    this->setupUser(&user);
}


void MainWindow::setupUser(Protocol::User *user, QString contactGroup)
{
    // try to get widgetitem
    QTreeWidgetItem* item = 0;
    if(this->mapUserItems.contains(user->id())) {
        item = this->mapUserItems.value(user->id());
    }

    // simplefy User
    qint32 userId = user->id();
    QString userUsername = QString::fromStdString(user->username());
    bool userOnline = user->online();

    // create on/offline icon
    QString strIconFilename;
    if(userOnline) {
        strIconFilename = ":/state/available";
    } else {
        strIconFilename = ":/state/offline";
    }
    QIcon iconContact(strIconFilename);

    // get/construct Top group treewidget
    QTreeWidgetItem *treeWidgetUser = !item ? new QTreeWidgetItem : item;
    if(!item) {
        QTreeWidgetItem *widgetItem = 0;
        if(this->mapGroups.contains(contactGroup)){
            widgetItem = this->mapGroups.value(contactGroup);
        } else {
            widgetItem = new QTreeWidgetItem;
            widgetItem->setText(0, contactGroup);
            this->mapGroups.insert(contactGroup, widgetItem);
            this->ui->treeWidgetContactList->addTopLevelItem(widgetItem);
        }

        // add new tree widget as child to group top level item
        widgetItem->addChild(treeWidgetUser);

        // add treewidget to the useritems map
        this->mapUserItems.insert(userId, treeWidgetUser);

        // set font size
        QFont font = treeWidgetUser->font(0);
        font.setPointSize(12);
        treeWidgetUser->setFont(0, font);

        // save user
        this->mapIdUser.insert(userId, new Protocol::User(*user));
    }

    // set contact data
    treeWidgetUser->setData(0, Qt::UserRole, userId);
    treeWidgetUser->setText(0, userUsername + strIconFilename);
    treeWidgetUser->setIcon(0, iconContact);
}

void MainWindow::onUserClicked(QTreeWidgetItem *widgetClicked, int column)
{
    Q_UNUSED(column);

    // get userid of clicked widget item
    qint32 userId = widgetClicked->data(0, Qt::UserRole).toInt();

    // exit if no UserRole data was set
    if(!userId) {
        return;
    }

    // get user
    Protocol::User *user = this->mapIdUser.value(userId);

    // add user to chatBox and show the chatBox
    this->chatBox->addNewUser(user);
}
