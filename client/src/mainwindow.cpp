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

    // make sure that the gui was complete constructed
    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);

    // signal --> slot connections (Socket)
    this->connect(Global::socketServer, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(serverConnectionError(QAbstractSocket::SocketError)));

    // signal --> slot connections (PacketProcessor)
    Global::eleaphRpc->registerRPCMethod("contactlist", this, SLOT(handleContactList(DataPacket*)));
    Global::eleaphRpc->registerRPCMethod("user_altered", this, SLOT(handleUserAltered(DataPacket*)));

    //this->connect(Global::packetProcessor, SIGNAL(userInformationsReceived(Protocol::UserInformations)), this, SLOT(userInformationsReceived(Protocol::UserInformations)));
    //this->connect(Global::packetProcessor, SIGNAL(userAltered(Protocol::User)), this, SLOT(contactListUserAltered(Protocol::User)));

    // handle double click event of new user
    this->connect(this->ui->treeWidgetContactList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onUserClicked(QTreeWidgetItem*,int)));
    this->connect(this->ui->actionLogout, SIGNAL(triggered()), this, SLOT(handleLogout()));
    this->connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(deleteLater()));
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

void MainWindow::handleContactList(DataPacket *dataPacket)
{
    // get contact list
    Protocol::ContactList contactList;
    contactList.ParseFromArray(dataPacket->baRawPacketData->data(), dataPacket->baRawPacketData->length());

    // remove all users form contact list
    this->ui->treeWidgetContactList->clear();

    // loop contacts in contact list and all needed gui elements
    for(int i = 0;i<contactList.contact_size();i++) {
        // simplefy contact
        Protocol::Contact *contact = contactList.mutable_contact(i);

        // setup user
        this->setupUser(contact->mutable_user(), QString::fromStdString(contact->group()));
    }

    // expand all groups
    this->ui->treeWidgetContactList->expandAll();
}

void MainWindow::handleUserAltered(DataPacket *dataPacket)
{
    Protocol::User user;
    user.ParseFromArray(dataPacket->baRawPacketData->data(), dataPacket->baRawPacketData->length());
    this->contactListUserAltered(user);
}

void MainWindow::handleLogout()
{
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
    // get userid of clicked widget item
    qint32 userId = widgetClicked->data(0, Qt::UserRole).toInt();

    // exit if no UserRole data was set
    if(!userId) {
        return;
    }

    // get user
    Protocol::User *user = this->mapIdUser.value(userId);

    // add user to chatBox and show the chatBox
    this->chatBox.addNewUser(user);
    this->chatBox.show();
}
