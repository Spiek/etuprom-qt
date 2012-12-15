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
    EleaphProtoRPC *eleaphRpc = Global::eleaphRpc;
    eleaphRpc->registerRPCMethod("message.private", this, SLOT(handleTextMessage(DataPacket*)));
}

ChatBox::~ChatBox()
{
    delete ui;
}

void ChatBox::closeEvent(QCloseEvent *closeEvent)
{
    closeEvent->ignore();
    this->hide();
}

void ChatBox::loadDesign(QString strDesign)
{
    // some global path pars
    QString strDesignPath = QString("design/%1/Contents/Resources/").arg(strDesign);

    // Global (css etc.)
    this->designCurrent.urlPathToMainCss = QUrl::fromLocalFile(strDesignPath + "main.css");

    QFile fileContentOfFooter(strDesignPath + "Footer.html");
    fileContentOfFooter.open(QFile::ReadOnly);
    this->designCurrent.strContentOfFooter = fileContentOfFooter.readAll();

    QFile fileContentOfHeader(strDesignPath + "Header.html");
    fileContentOfHeader.open(QFile::ReadOnly);
    this->designCurrent.strContentOfHeader = fileContentOfHeader.readAll();

    QFile fileContentOfStatus(strDesignPath + "Status.html");
    fileContentOfStatus.open(QFile::ReadOnly);
    this->designCurrent.strContentOfStatus = fileContentOfStatus.readAll();


    // Incoming html
    QFile fileContentOfInBoundAction(strDesignPath + "Incoming/Action.html");
    fileContentOfInBoundAction.open(QFile::ReadOnly);
    this->designCurrent.strContentOfInBoundAction = fileContentOfInBoundAction.readAll();

    QFile fileContentOfInBoundContent(strDesignPath + "Incoming/Content.html");
    fileContentOfInBoundContent.open(QFile::ReadOnly);
    this->designCurrent.strContentOfInBoundContent = fileContentOfInBoundContent.readAll();

    QFile fileContentOfInBoundNextContent(strDesignPath + "Incoming/NextContent.html");
    fileContentOfInBoundNextContent.open(QFile::ReadOnly);
    this->designCurrent.strContentOfInBoundNextContent = fileContentOfInBoundNextContent.readAll();


    // Outgoing html
    QFile fileContentOfOutBoundAction(strDesignPath + "Outgoing/Action.html");
    fileContentOfOutBoundAction.open(QFile::ReadOnly);
    this->designCurrent.strContentOfOutBoundAction = fileContentOfOutBoundAction.readAll();

    QFile fileContentOfOutBoundContent(strDesignPath + "Outgoing/Content.html");
    fileContentOfOutBoundContent.open(QFile::ReadOnly);
    this->designCurrent.strContentOfOutBoundContent = fileContentOfOutBoundContent.readAll();

    QFile fileContentOfOutBoundNextContent(strDesignPath + "Outgoing/NextContent.html");
    fileContentOfOutBoundNextContent.open(QFile::ReadOnly);
    this->designCurrent.strContentOfOutBoundNextContent = fileContentOfOutBoundNextContent.readAll();

    foreach(Ui_FormChatWidget* chatFormUser, this->mapUserIdChatForm.values()) {
        chatFormUser->webView->settings()->setUserStyleSheetUrl(this->designCurrent.urlPathToMainCss);
    }
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

        // set style sheets
        chatForm->webView->settings()->setUserStyleSheetUrl(this->designCurrent.urlPathToMainCss);

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
        Global::eleaphRpc->sendRPCDataPacket(Global::socketServer, "message.private", message.SerializeAsString());

        // add message
        this->addMessage(strMessage, Global::mapCachedUsers.value(userId), false, QDateTime::currentDateTime().toTime_t());

        // clear message
        chatForm->plainTextEditText->clear();
    }
}

void ChatBox::handleTextMessage(DataPacket *dataPacket)
{
    // parse protocol
    Protocol::MessagePrivateServer message;
    if(!message.ParseFromArray(dataPacket->baRawPacketData->constData(), dataPacket->baRawPacketData->length())) {
        qWarning("[%s][%d] - Protocol Violation by Trying to Parse MessagePrivateServer", __PRETTY_FUNCTION__ , __LINE__);
        return;
    }

    // add message
    return this->addMessage(QString::fromStdString(message.text()), message.mutable_usersender(), true, message.timestamp());
}

// true -> Inbound
// false -> Outbound
void ChatBox::addMessage(QString text, Protocol::User* user, bool direction, quint32 timeStamp)
{
    // if we have a message, and the chat window for the sending user doesn't exist, create it
    if(!this->mapUserIdChatForm.contains(user->id())) {
        // if user doen't exist in cached list, add user to cached list
        if(!Global::mapCachedUsers.contains(user->id())) {
            Global::mapCachedUsers.insert(user->id(), new Protocol::User(*user));
        }

        // add new user first
        this->addNewUser(user);
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
        strNewTextMessage = *boolLastMessageDirection ? this->designCurrent.strContentOfOutBoundNextContent : this->designCurrent.strContentOfOutBoundContent;
    }

    // otherwise create inbound html code, if we have a inbound message (Remote User --> Logged in User)
    else {
        strNewTextMessage = !*boolLastMessageDirection ? this->designCurrent.strContentOfInBoundNextContent : this->designCurrent.strContentOfInBoundContent;
    }

    // replace values
    strNewTextMessage = strNewTextMessage.replace("%sender%", QString::fromStdString(direction ? user->username() : Global::user->username()));
    strNewTextMessage = strNewTextMessage.replace("%time{%H:%M}%", QDateTime::fromTime_t(timeStamp).toString("hh:mm"));
    strNewTextMessage = strNewTextMessage.replace("%message%", text);

    // update html code, and scrollbar
    QString strHtml = chatWidget->webView->page()->mainFrame()->toHtml();
    chatWidget->webView->page()->mainFrame()->setHtml(strHtml + strNewTextMessage);
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
