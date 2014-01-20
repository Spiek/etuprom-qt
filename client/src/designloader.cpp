#include "designloader.h"

DesignLoader::ChatDesign DesignLoader::loadChatDesign(QString strDesign)
{
    // create some global path pars
    QString strDesignPath = QString("design/%1/ChatBox/").arg(strDesign);
    DesignLoader::ChatDesign design;
    design.urlDesigndir = QUrl::fromLocalFile(QDir(strDesignPath).absolutePath() + "/");

    // load "global" design configuration
    design.strMainHtml = DesignLoader::readFileContent(QString("%1/main.html").arg(strDesignPath));

    // load "incoming" design configuration (if incoming_first.html doesn't exist, take incoming_next.html as incoming first!)
    if(QFile::exists(QString("%1/incoming_first.html").arg(strDesignPath))) {
        design.strIncomingFirst = DesignLoader::readFileContent(QString("%1/incoming_first.html").arg(strDesignPath));
    } else {
        design.strIncomingFirst = DesignLoader::readFileContent(QString("%1/incoming_next.html").arg(strDesignPath));
    }
    design.strIncomingNext = DesignLoader::readFileContent(QString("%1/incoming_next.html").arg(strDesignPath));

    // load "outgoing" design configuration (if outgoing_first.html doesn't exist, take outgoing_next.html as outgoing first!)
    if(QFile::exists(QString("%1/outgoing_first.html").arg(strDesignPath))) {
        design.strOutgoingFirst = DesignLoader::readFileContent(QString("%1/outgoing_first.html").arg(strDesignPath));
    } else {
        design.strOutgoingFirst = DesignLoader::readFileContent(QString("%1/outgoing_next.html").arg(strDesignPath));
    }
    design.strOutgoingNext = DesignLoader::readFileContent(QString("%1/outgoing_next.html").arg(strDesignPath));

    // return fully constructed design
    return design;
}


QString DesignLoader::readFileContent(QString strFile)
{
    QFile file(strFile);
    file.open(QFile::ReadOnly);
    return file.readAll();
}
