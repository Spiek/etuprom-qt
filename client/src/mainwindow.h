/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt (gui)
#include <QtWidgets/QMainWindow>
#include <QtGui/QIcon>
#include <QtWidgets/QTreeWidgetItem>

// own (client)
#include "global.h"

// own (gui)
#include "loginform.h"
#include "chatbox.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
        QMap<qint32, Protocol::User*> mapIdUser;
        QMap<qint32, QTreeWidgetItem*> mapUserItems;
        QMap<QString, QTreeWidgetItem*> mapGroups;

        // helper functions
        void requestUserInformations();
        void setupUser(Protocol::User *user, QString contactGroup = "");

        // designer members
        ChatBox* chatBox;

    private slots:
        // Socket slots
        void serverConnectionError(QAbstractSocket::SocketError socketError);
        void contactListUserAltered(Protocol::User user);

        // GUI slots
        void onUserClicked(QTreeWidgetItem* widgetClicked, int column);
        void constructContactList();
        void setupLoggedInUser();

        // Protocol slots (server --> client)
        void handleUserAltered(DataPacket *dataPacket);

        // Protocol slots (client --> server)
        void handleLogout();
};

#endif // MAINWINDOW_H
