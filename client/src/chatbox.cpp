/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "chatbox.h"
#include "ui_chatbox.h"
#include "ui_chatTab.h"

ChatBox::ChatBox(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatBox)
{
    ui->setupUi(this);

    // handle text edits
    this->sigMapperUserMessages = new QSignalMapper(this);
    this->connect(this->sigMapperUserMessages, SIGNAL(mapped(int)), this, SLOT(chatTextChanged(int)));

    // protocol handlings
    EleaphProtoRPC *eleaphRpc = Global::eleaphRpc;
    eleaphRpc->registerRPCMethod("message.private", this, SLOT(handleTextMessage(DataPacket*)));
}

ChatBox::~ChatBox()
{
    delete ui;
}

void ChatBox::addNewUser(Protocol::User *user)
{
    // if tab for user not exist, create it
    int intTabIndex = 0;
    if(!this->mapUserIdTabIndex.contains(user->id())) {
        // create new chatTab widget, by using created chatTab.ui
        QWidget* newTab = new QWidget(this);
        Ui_FormChatWidget *chatForm = new Ui_FormChatWidget;
        chatForm->setupUi(newTab);

        // handle text changing event from users textedit
        this->connect(chatForm->plainTextEditText, SIGNAL(textChanged()), this->sigMapperUserMessages, SLOT(map()));
        sigMapperUserMessages->setMapping(chatForm->plainTextEditText, user->id());

        // add tab and save user
        intTabIndex = this->ui->tabWidget->addTab(newTab, QString::fromStdString(user->username()));
        this->mapUserIdTabIndex.insert(user->id(), intTabIndex);
        this->mapUserIdChatForm.insert(user->id(), chatForm);
    }

    // focus the user tab
    this->ui->tabWidget->setCurrentIndex(!intTabIndex ? this->mapUserIdTabIndex.value(user->id()) : intTabIndex);
}

void ChatBox::chatTextChanged(int userId)
{
    // get chatWidget for user
    Ui_FormChatWidget *chatForm = (Ui_FormChatWidget*)this->mapUserIdChatForm.value(userId);

    // if user has pressed the enter button, communicate message to outside
    QString strMessage = chatForm->plainTextEditText->toPlainText();
    if(strMessage.right(1) == "\n") {
        strMessage.remove(strMessage.length() - 1, 1);

        // send message to server
        Protocol::MessagePrivate message;
        message.set_useridsenderreceiver(userId);
        message.set_text(strMessage.toStdString());
        message.set_timestamp(QDateTime::currentMSecsSinceEpoch() / 1000);
        Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, "message.private", message.SerializeAsString());

        // clear message
        chatForm->plainTextEditText->clear();
    }
}

void ChatBox::handleTextMessage(DataPacket *dataPacket)
{
    // parse protocol
    Protocol::MessagePrivate message;
    if(!message.ParseFromArray(dataPacket->baRawPacketData->constData(), dataPacket->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse MessagePrivate", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }

    // get chat widget
    Ui_FormChatWidget *chatForm = (Ui_FormChatWidget*)this->mapUserIdChatForm.value(message.useridsenderreceiver());
    chatForm->textBrowser->insertHtml(QString("<b>[%1]%2</b> %3<br />").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"), QString::number(message.useridsenderreceiver()), QString::fromStdString(message.text())));
}
