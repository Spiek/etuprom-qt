/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef SQMPACKETPROCESSOR_H
#define SQMPACKETPROCESSOR_H

// Qt (core)
#include <QObject>

// own (collective)
#include "EleaphProtoRpc"
#include "protocol.pb.h"

// own
#include "global.h"

class SQMPacketProcessor : public QObject
{
    Q_OBJECT
    public:
        SQMPacketProcessor(QObject *parent = 0);

    private:
        // members
        bool loggedIn;
};


#endif // SQMPACKETPROCESSOR_H
