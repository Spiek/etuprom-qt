#ifndef CHATBOX_H
#define CHATBOX_H

#include <QMainWindow>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>

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

    private slots:
        void chatTextChanged(int userId);

    private:
        QMap<qint32, qint32> mapUserIdTabIndex;
        QMap<qint32, void*> mapUserIdChatForm;
        Ui::ChatBox *ui;
        QSignalMapper *sigMapperUserMessages;
};

#endif // CHATBOX_H
