#ifndef CHATBOX_H
#define CHATBOX_H

#include <QMainWindow>
#include <QtCore/QMap>

// own (protbuf)
#include "protocol.pb.h"

namespace Ui {
    class ChatBox;
}

class ChatBox : public QMainWindow
{
    Q_OBJECT

    public:
        ChatBox(QWidget *parent = 0);
        ~ChatBox();

        // extern slot class accessors
        void addNewUser(Protocol::User *user);

    private:
        QMap<int, int> mapUserIdTabIndex;
        Ui::ChatBox *ui;
};

#endif // CHATBOX_H
