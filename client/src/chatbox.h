/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef CHATBOX_H
#define CHATBOX_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>
#include <QtCore/QDateTime>
#include <QtCore/QFile>

// gui
#include <QtWebKitWidgets/QWebFrame>

// own
#include "designloader.h"
#include "global.h"
#include "ui_chatTab.h"

// own (protbuf)
#include "protocol.pb.h"
#include "collective/proto/packettypes.h"

namespace Ui {
    class ChatBox;
}

class ChatBox : public QMainWindow
{
    Q_OBJECT
    public:
        ChatBox(QWidget *parent = 0);
        ~ChatBox();

    public slots:
        // Extern class accessor slots
        void showUserChatBox(Protocol::User *user);
        void addMessage(QString text, int userId, bool direction, quint32 timeStamp);
        void loadDesign(QString strDesign);

    private slots:
        // Gui slots
        void chatTextChanged(int userId);

        // Protocol handlers
        void handleInboundTextMessage(EleaphRpcPacket dataPacket);

    protected:
        // Overriden Parent functions
        void closeEvent(QCloseEvent *closeEvent);

    private:
        // design components
        DesignLoader::ChatDesign designChat;

        // tab mapping
        QMap<qint32, qint32> mapUserIdTabIndex;
        QMap<qint32, Ui_FormChatWidget*> mapUserIdChatForm;
        Ui::ChatBox *ui;
        QSignalMapper *sigMapperUserMessages;
        QMap<qint32, bool*> mapUserLastMessage;
};

#endif // CHATBOX_H
