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
        this->newMessage(userId, strMessage);

        // clear message
        chatForm->plainTextEditText->clear();
    }
}
