#ifndef DESIGNLOADER_H
#define DESIGNLOADER_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QFile>
#include <QFileInfo>


class DesignLoader
{
    public:
        struct ChatDesign {
            // Global
            QUrl urlDesigndir;
            QString strMainHtml;

            // Incoming html
            QString strIncomingFirst;
            QString strIncomingNext;

            // Incoming html
            QString strOutgoingFirst;
            QString strOutgoingNext;
        };

        static DesignLoader::ChatDesign loadChatDesign(QString strDesign);

    private:
        static QString readFileContent(QString strFile);
};

#endif // CHATDESIGNLOADER_H
