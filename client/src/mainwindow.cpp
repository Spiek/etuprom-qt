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
    this->connect(Global::packetProcessor, SIGNAL(userInformationsReceived(Protocol::UserInformations)), this, SLOT(userInformationsReceived(Protocol::UserInformations)));
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

void MainWindow::userInformationsReceived(Protocol::UserInformations userInformations)
{
    // toptreewidget items for the group assigment
    QMap<QString, QTreeWidgetItem*> mapTopTreeWidgetItems;

    // create all needed gui elements for contacts
    for(int i = 0;i<userInformations.contact_size();i++) {
        // simplefy contact
        Protocol::Contact contact = userInformations.contact(i);
        qint32 contactId = contact.id();
        QString contactUserName = QString::fromStdString(contact.username());
        Protocol::Contact::State contactState = contact.state();
        QString contactGroup = QString::fromStdString(contact.group());

        // create state icon
        QString strIconFilename;
        if(contactState == Protocol::Contact::Online) {
            strIconFilename = ":/state/available";
        } else if(contactState == Protocol::Contact::Offline) {
            strIconFilename = ":/state/offline";
        }
        QIcon iconContact(strIconFilename);

        // get/construct Top treewidget
        QTreeWidgetItem *widgetItem = 0;
        if(mapTopTreeWidgetItems.contains(contactGroup)){
            widgetItem = mapTopTreeWidgetItems.value(contactGroup);
        } else {
            widgetItem = new QTreeWidgetItem;
            this->ui->treeWidgetContactList->addTopLevelItem(widgetItem);
            widgetItem->setText(0, contactGroup);
            mapTopTreeWidgetItems.insert(contactGroup, widgetItem);
        }

        // set contact data
        QTreeWidgetItem *treeWidgetUser = new QTreeWidgetItem;
        treeWidgetUser->setData(0, Qt::UserRole, contactId);
        treeWidgetUser->setText(0, contactUserName);
        treeWidgetUser->setIcon(0, iconContact);
        widgetItem->addChild(treeWidgetUser);
    }
}
