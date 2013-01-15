#include "designloader.h"

DesignLoader::ChatDesign DesignLoader::loadChatDesign(QString strDesign)
{
    // some global path pars
    QString strDesignPath = QString("design/%1/Contents/Resources/").arg(strDesign);
    DesignLoader::ChatDesign design;

    // Global (css etc.)
    design.urlPathToMainCss = QUrl::fromLocalFile(strDesignPath + "main.css");

    QFile fileContentOfFooter(strDesignPath + "Footer.html");
    fileContentOfFooter.open(QFile::ReadOnly);
    design.strContentOfFooter = fileContentOfFooter.readAll();

    QFile fileContentOfHeader(strDesignPath + "Header.html");
    fileContentOfHeader.open(QFile::ReadOnly);
    design.strContentOfHeader = fileContentOfHeader.readAll();

    QFile fileContentOfStatus(strDesignPath + "Status.html");
    fileContentOfStatus.open(QFile::ReadOnly);
    design.strContentOfStatus = fileContentOfStatus.readAll();


    // Incoming html
    QFile fileContentOfInBoundAction(strDesignPath + "Incoming/Action.html");
    fileContentOfInBoundAction.open(QFile::ReadOnly);
    design.strContentOfInBoundAction = fileContentOfInBoundAction.readAll();

    QFile fileContentOfInBoundContent(strDesignPath + "Incoming/Content.html");
    fileContentOfInBoundContent.open(QFile::ReadOnly);
    design.strContentOfInBoundContent = fileContentOfInBoundContent.readAll();

    QFile fileContentOfInBoundNextContent(strDesignPath + "Incoming/NextContent.html");
    fileContentOfInBoundNextContent.open(QFile::ReadOnly);
    design.strContentOfInBoundNextContent = fileContentOfInBoundNextContent.readAll();


    // Outgoing html
    QFile fileContentOfOutBoundAction(strDesignPath + "Outgoing/Action.html");
    fileContentOfOutBoundAction.open(QFile::ReadOnly);
    design.strContentOfOutBoundAction = fileContentOfOutBoundAction.readAll();

    QFile fileContentOfOutBoundContent(strDesignPath + "Outgoing/Content.html");
    fileContentOfOutBoundContent.open(QFile::ReadOnly);
    design.strContentOfOutBoundContent = fileContentOfOutBoundContent.readAll();

    QFile fileContentOfOutBoundNextContent(strDesignPath + "Outgoing/NextContent.html");
    fileContentOfOutBoundNextContent.open(QFile::ReadOnly);
    design.strContentOfOutBoundNextContent = fileContentOfOutBoundNextContent.readAll();

    // return fully constructed design
    return design;
}
