/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "chatbox.h"
#include "ui_chatbox.h"

ChatBox::ChatBox(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatBox)
{
    ui->setupUi(this);

    // handle text edits
    this->sigMapperUserMessages = new QSignalMapper(this);
    this->connect(this->sigMapperUserMessages, SIGNAL(mapped(int)), this, SLOT(chatTextChanged(int)));

    // protocol handlings
    Global::eleaphRpc->registerRPCMethod(PACKET_DESCRIPTOR_CHAT_PRIVATE, this, SLOT(handleInboundTextMessage(DataPacket*)));
}

ChatBox::~ChatBox()
{
    delete ui;
    delete this->sigMapperUserMessages;
}


//
// Extern class accessor slots
//

void ChatBox::showUserChatBox(Protocol::User *user)
{
    // if tab for user not exist, create it
    int intTabIndex = 0;
    if(!this->mapUserIdTabIndex.contains(user->id())) {
        // create new chatTab widget, by using created chatTab.ui
        QWidget* newTab = new QWidget(this);
        Ui_FormChatWidget *chatForm = new Ui_FormChatWidget;
        chatForm->setupUi(newTab);

        // set style sheets
        chatForm->webView->setHtml(this->designChat.strMainHtml, this->designChat.urlDesigndir);

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
    this->show();
}

void ChatBox::addMessage(QString text, int userId, bool direction, quint32 timeStamp)
{
    // direction:
    // true --> Inbound
    // false --> Outbound

    // get user object (if present localy, otherwise request it from server!)
    if(!Global::mapCachedUsers.contains(userId)) {
        // FIXME! implement user.get
        return;
    }
    Protocol::User* user = Global::mapCachedUsers.value(userId);

    // if we have a message, and the chat window for the sending user doesn't exist, create it
    if(!this->mapUserIdChatForm.contains(user->id())) {
        // if user doen't exist in cached list, add user to cached list
        if(!Global::mapCachedUsers.contains(user->id())) {
            Global::mapCachedUsers.insert(user->id(), new Protocol::User(*user));
        }

        // add new user first
        this->showUserChatBox(user);
    }

    // get last message direction
    bool* boolLastMessageDirection = 0;
    if(!this->mapUserLastMessage.contains(user->id())) {
        this->mapUserLastMessage.insert(user->id(), (bool*)malloc(sizeof(bool)));
        boolLastMessageDirection = this->mapUserLastMessage.value(user->id());
        *boolLastMessageDirection = direction;
    } else {
        boolLastMessageDirection = this->mapUserLastMessage.value(user->id());
    }
    // simplefy some values
    Ui_FormChatWidget* chatWidget = this->mapUserIdChatForm.value(user->id());
    QString strNewTextMessage;

    // create outbound html code, if we have a outbound message (Logged in User --> Remote User)
    if(!direction) {
        strNewTextMessage = *boolLastMessageDirection ? this->designChat.strOutgoingNext : this->designChat.strOutgoingFirst;
    }

    // otherwise create inbound html code, if we have a inbound message (Remote User --> Logged in User)
    else {
        strNewTextMessage = !*boolLastMessageDirection ? this->designChat.strIncomingNext : this->designChat.strIncomingFirst;
    }

    // replace values
    strNewTextMessage = strNewTextMessage.replace("<!-- %%sender%% -->", QString::fromStdString(direction ? user->username() : Global::user->username()));
    strNewTextMessage = strNewTextMessage.replace("<!-- %%time{%H:%M:%S}%% -->", QDateTime::fromTime_t(timeStamp).toString("hh:mm:ss"));
    strNewTextMessage = strNewTextMessage.replace("<!-- %%message%% -->", text);

    // update html code, and scrollbar
    QString strHtml = chatWidget->webView->page()->mainFrame()->toHtml();
    strHtml = strHtml.replace("<!-- %%chatcontent %% -->", strNewTextMessage + "<!-- %%chatcontent %% -->");

    chatWidget->webView->setHtml(strHtml, this->designChat.urlDesigndir);
    chatWidget->webView->page()->mainFrame()->setScrollBarValue(Qt::Vertical, chatWidget->webView->page()->mainFrame()->scrollBarMaximum(Qt::Vertical));

    // get last message direction
    if(direction == *boolLastMessageDirection) {
        *boolLastMessageDirection = !*boolLastMessageDirection;
    }

    // focus the user tab
    this->ui->tabWidget->setCurrentIndex(this->mapUserIdTabIndex.value(user->id()));

    // show the form
    this->show();
}

void ChatBox::loadDesign(QString strDesign)
{
    this->designChat = DesignLoader::loadChatDesign(strDesign);

    // install design
    foreach(Ui_FormChatWidget* chatFormUser, this->mapUserIdChatForm.values()) {
        chatFormUser->webView->setHtml(this->designChat.strMainHtml, this->designChat.urlDesigndir);
    }
}


//
// Gui slots
//

void ChatBox::chatTextChanged(int userId)
{
    // get chatWidget for user
    Ui_FormChatWidget *chatForm = this->mapUserIdChatForm.value(userId);

    // if user has pressed the enter button, communicate message to outside
    QString strMessage = chatForm->plainTextEditText->toPlainText();
    if(strMessage.right(1) == "\n") {
        strMessage.remove(strMessage.length() - 1, 1);

        // send message to server
        Protocol::MessagePrivateClient message;
        message.set_useridreceiver(userId);
        message.set_text(strMessage.toStdString());
        Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, PACKET_DESCRIPTOR_CHAT_PRIVATE, message.SerializeAsString());

        // add message
        this->addMessage(strMessage, userId, false, QDateTime::currentDateTime().toTime_t());

        // clear message
        chatForm->plainTextEditText->clear();
    }
}


//
// Protocol handlers
//

void ChatBox::handleInboundTextMessage(EleaphRpcPacket dataPacket)
{
    // parse protocol
    Protocol::MessagePrivateServer message;
    if(!message.ParseFromArray(dataPacket.data()->baRawPacketData->constData(), dataPacket.data()->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse MessagePrivateServer", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }

    // add message
    return this->addMessage(QString::fromStdString(message.text()), message.userid(), message.direction() == Protocol::MessagePrivateServer_Receiver_Target, message.timestamp());
}


//
// Overriden Parent functions
//
void ChatBox::closeEvent(QCloseEvent *closeEvent)
{
    // it's not possible to really close the form
    closeEvent->ignore();
    this->hide();
}
