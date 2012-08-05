#include "chatbox.h"
#include "ui_chatbox.h"
#include "ui_chatTab.h"

ChatBox::ChatBox(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatBox)
{
    ui->setupUi(this);




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
        Ui_FormChatWidget *chatWidget = new Ui_FormChatWidget;
        chatWidget->setupUi(newTab);

        // add tab and save user
        intTabIndex = this->ui->tabWidget->addTab(newTab, QString::fromStdString(user->username()));
        this->mapUserIdTabIndex.insert(user->id(), intTabIndex);
    }

    // focus the user tab
    this->ui->tabWidget->setCurrentIndex(!intTabIndex ? this->mapUserIdTabIndex.value(user->id()) : intTabIndex);
}
