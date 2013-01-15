#ifndef DESIGNLOADER_H
#define DESIGNLOADER_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QFile>

class DesignLoader
{
    public:
        struct ChatDesign {
            // Global
            QUrl urlPathToMainCss;

            // Global html
            QString strContentOfFooter;
            QString strContentOfHeader;
            QString strContentOfStatus;

            // Incoming html
            QString strContentOfInBoundAction;
            QString strContentOfInBoundContent;
            QString strContentOfInBoundNextContent;

            // Outgoing html
            QString strContentOfOutBoundAction;
            QString strContentOfOutBoundContent;
            QString strContentOfOutBoundNextContent;
        };

        static DesignLoader::ChatDesign loadChatDesign(QString strDesign);
};

#endif // CHATDESIGNLOADER_H
