/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef CHATBOX_H
#define CHATBOX_H

#include <QMainWindow>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>
#include <QtCore/QDateTime>
#include <QtCore/QFile>

// gui
#include <QWebFrame>

// own
#include "designloader.h"
#include "global.h"
#include "ui_chatTab.h"

// own (protbuf)
#include "protocol.pb.h"

namespace Ui {
    class ChatBox;
}

class ChatBox : public QMainWindow
{
    Q_OBJECT

    signals:
        void newMessage(qint32 userIdReceiver, QString strMessage);

    public:
        ChatBox(QWidget *parent = 0);
        ~ChatBox();


    public slots:
        // extern slot class accessors
        void addNewUser(Protocol::User *user);
        void loadDesign(QString strDesign);
        void addMessage(QString text, Protocol::User* user, bool direction, quint32 timeStamp);

    private slots:
        void chatTextChanged(int userId);

        // protocol handlers
        void handleTextMessage(DataPacket *dataPacket);

    protected:
        void closeEvent(QCloseEvent *closeEvent);

    private:
        DesignLoader::ChatDesign designChat;

        QMap<qint32, qint32> mapUserIdTabIndex;
        QMap<qint32, Ui_FormChatWidget*> mapUserIdChatForm;
        Ui::ChatBox *ui;
        QSignalMapper *sigMapperUserMessages;

        // design components
        QMap<qint32, bool*> mapUserLastMessage;
};

#endif // CHATBOX_H
