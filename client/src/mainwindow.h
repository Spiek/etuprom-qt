/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt (gui)
#include <QtGui/QMainWindow>
#include <QtGui/QIcon>
#include <QtGui/QTreeWidgetItem>

// own (client)
#include "global.h"

// own (gui)
#include "loginform.h"


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
        QMap<qint32, QTreeWidgetItem*> mapUserItems;
        QMap<QString, QTreeWidgetItem*> mapGroups;

        // helper functions
        void requestUserInformations();
        void setupUser(Protocol::User *user, QString contactGroup = "");

    private slots:
        // Socket slots
        void serverConnectionError(QAbstractSocket::SocketError socketError);
        void userInformationsReceived(Protocol::UserInformations userInformations);
        void contactListUserAltered(Protocol::User user);
};

#endif // MAINWINDOW_H
